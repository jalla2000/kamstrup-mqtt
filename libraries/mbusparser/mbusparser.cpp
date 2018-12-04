#include "mbusparser.h"
#include <cassert>

size_t VectorView::find(const std::vector<uint8_t>& needle) const
{
  for (const uint8_t* it = m_start; it < (m_start+m_size-needle.size()); ++it) {
    if (memcmp(it, &needle[0], needle.size()) == 0) {
      return it-m_start;
    }
  }
  return -1;
}

std::vector<VectorView> getFrames(const std::vector<uint8_t>& data)
{
  std::vector<VectorView> result;
  if (data.size() < 3) {
    return result;
  }
  size_t offset = 0;
  while (true) {
    //std::cout << "Now searching from offset=" << offset << std::endl;
    size_t start = 0;
    bool startFound = false;
    uint16_t messageSize = 0;
    for (size_t i = offset; i < data.size()-1; ++i) {
      if (data[i] == 0x7E && (data[i+1]&0xF0) == 0xA0) {
	startFound = true;
	start = i;
	messageSize = (data[i+1]&0x0F << 8) | data[i+2];
	break;
      }
    }
    if (!startFound) {
      return result;
    }
    size_t end = 0;
    bool endFound = false;
    if (messageSize < data.size()-start) {
      if (data[start+messageSize+1] == 0x7E) {
	endFound = true;
	end = start+messageSize+2;
      }
    }
    if (startFound && endFound && (end-start > 2)) {
      //std::cout << "Start: " << start << " End: " << end << std::endl;
      result.push_back(VectorView(data, start, end-start));
    }
    offset = end+1;
  }
  return result;
}

uint32_t getObisValue(const VectorView& frame,
		      uint8_t codeA,
		      uint8_t codeB,
		      uint8_t codeC,
		      uint8_t codeD,
		      uint8_t codeE,
		      uint8_t codeF,
		      uint8_t size,
		      bool & success)
{
  success = false;
  std::vector<uint8_t> theObis = { 0x09, 0x06, codeA, codeB, codeC, codeD, codeE, codeF };
  int indexOfData = frame.find(theObis);
  if (indexOfData >= 0) {
    //std::cout << "IndexOfActivePower: " << indexOfData << std::endl;
    const uint8_t * theBytes = &(frame[0]) + indexOfData + theObis.size() + 1;
    // for (int i = 0; i < 20; ++i) {
    //   std::cout << "Byte " << std::hex << i << ": " << (int)theBytes[i] << std::dec << std::endl;
    // }
    if (size == 2) {
      success = true;
      return
	(uint32_t)theBytes[0] << (8*1) |
	(uint32_t)theBytes[1];
    } else if (size == 3) {
      success = true;
      return
	(uint32_t)theBytes[0] << (8*2) |
	(uint32_t)theBytes[1] << (8*1) |
	(uint32_t)theBytes[2];
    } else if (size == 4) {
      success = true;
      return
	(uint32_t)theBytes[0] << (8*3) |
	(uint32_t)theBytes[1] << (8*2) |
	(uint32_t)theBytes[2] << (8*1) |
	(uint32_t)theBytes[3];
    }
  }
  return 0;
}

enum PowerType {
  ACTIVE_POWER_PLUS = 1,
  ACTIVE_POWER_MINUS = 2,
  REACTIVE_POWER_PLUS = 3,
  REACTIVE_POWER_MINUS = 4,
  CURRENT_L1 = 31, // 0x1F
  CURRENT_L2 = 51, // 0x33
  CURRENT_L3 = 71, // 0x47
  VOLTAGE_L1 = 32, // 0x20
  VOLTAGE_L2 = 52, // 0x34
  VOLTAGE_L3 = 72 // 0x48
};

uint32_t getPower(const VectorView& frame,
                  PowerType type,
                  bool& success)
{
  switch (type) {
  case VOLTAGE_L1:
  case VOLTAGE_L2:
  case VOLTAGE_L3:
    return getObisValue(frame, 1, 1, type, 7, 0, 0xff, 2, success);
  default:
    return getObisValue(frame, 1, 1, type, 7, 0, 0xff, 4, success);
  }
}

MeterData parseMbusFrame(const VectorView& frame)
{
  MeterData result;
  unsigned int frameFormat = frame[1] & 0xF0;
  //std::cout << "Frame format: " << (int)frameFormat << std::endl;
  size_t messageSize = ((frame[1] & 0x0F) << 8) | frame[2];
  //std::cout << "Message size: " << (uint32_t)messageSize << std::endl;
  result.parseResultBufferSize = frame.size();
  result.parseResultMessageSize = messageSize;

  if (frame.front() == 0x7E && frame.back() == 0x7E) {
    if (frameFormat == 0xA0) {
      // TODO: Parse header
      // TODO: Parse datetime
      // TODO: Parse elements sequentially
      result.activePowerPlus = getPower(frame, ACTIVE_POWER_PLUS, result.activePowerPlusValid);
      result.activePowerMinus = getPower(frame, ACTIVE_POWER_MINUS, result.activePowerMinusValid);
      result.reactivePowerPlus = getPower(frame, REACTIVE_POWER_PLUS, result.reactivePowerPlusValid);
      result.reactivePowerMinus = getPower(frame, REACTIVE_POWER_MINUS, result.reactivePowerMinusValid);
      result.centiAmpereL1 = getPower(frame, CURRENT_L1, result.centiAmpereL1Valid);
      result.centiAmpereL2 = getPower(frame, CURRENT_L2, result.centiAmpereL2Valid);
      result.centiAmpereL3 = getPower(frame, CURRENT_L3, result.centiAmpereL3Valid);
      result.voltageL1 = getPower(frame, VOLTAGE_L1, result.voltageL1Valid);
      result.voltageL2 = getPower(frame, VOLTAGE_L2, result.voltageL2Valid);
      result.voltageL3 = getPower(frame, VOLTAGE_L3, result.voltageL3Valid);
    }
  }
  return result;
}

MbusStreamParser::MbusStreamParser(uint8_t* buf, size_t bufsize)
  : m_buf(buf)
  , m_bufsize(bufsize)
  , m_frameFound(nullptr, 0)
{
}

bool MbusStreamParser::pushData(uint8_t data)
{
  if (m_position >= m_bufsize) {
    m_position = 0;
    m_parseState = LOOKING_FOR_START;
    m_messageSize = 0;
  }
  if (m_position == 0) {
    m_frameFound = VectorView(nullptr, 0);
  }
  switch (m_parseState) {
  case LOOKING_FOR_START:
    if (data == 0x7E) {
      //std::cout << "Found frame start" << std::endl;
      m_parseState = LOOKING_FOR_FORMAT_TYPE;
      m_buf[m_position++] = data;
    }
    break;
  case LOOKING_FOR_FORMAT_TYPE:
    if ((data & 0xF0) == 0xA0) {
      //std::cout << "Found frame format type" << std::endl;
      m_parseState = LOOKING_FOR_SIZE;
      m_buf[m_position++] = data;
    }
    break;
  case LOOKING_FOR_SIZE:
    assert(m_position > 0);
    m_messageSize = (m_buf[m_position-1] & 0x0F) | data;
    //std::cout << "Message size is: " << m_messageSize << std::endl;
    m_parseState = LOOKING_FOR_END;
    m_buf[m_position++] = data;
    break;
  case LOOKING_FOR_END:
    m_buf[m_position++] = data;
    if (m_position == (m_messageSize+2)) {
      if (data == 0x7E) {
        m_frameFound = VectorView(m_buf, m_position);
        m_parseState = LOOKING_FOR_START;
        m_position = 0;
        return true;
      } else {
        //std::cout << "Byte at end position: " << std::hex << (unsigned)data << std::dec << std::endl;
        m_parseState = LOOKING_FOR_START;
        m_position = 0;
      }
    }
    break;
  }
  return false;
}

VectorView MbusStreamParser::getFrame()
{
  return m_frameFound;
}
