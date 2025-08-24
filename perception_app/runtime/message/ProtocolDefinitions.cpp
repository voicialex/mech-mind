#include "ProtocolDefinitions.hpp"
#include <algorithm>

namespace perception {

uint16_t ProtocolUtils::CalculateCRC16(const std::vector<uint8_t> &data) {
  return CalculateCRC16(data.data(), data.size());
}

uint16_t ProtocolUtils::CalculateCRC16(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc = crc >> 1;
      }
    }
  }

  return crc;
}

bool ProtocolUtils::VerifyCRC16(const std::vector<uint8_t> &data, uint16_t crc16) {
  return CalculateCRC16(data) == crc16;
}

ProtocolFrame ProtocolUtils::ParseFrame(const std::vector<uint8_t> &data) {
  ProtocolFrame frame;

  if (data.size() >= ProtocolConstants::HEADER_SIZE) {
    // 解析魔术字 (2字节)
    frame.magic_id = static_cast<uint16_t>(data[ProtocolConstants::MAGIC_OFFSET]) |
                     (static_cast<uint16_t>(data[ProtocolConstants::MAGIC_OFFSET + 1]) << 8);

    // 解析CRC16 (2字节)
    frame.crc16 = static_cast<uint16_t>(data[ProtocolConstants::CRC16_OFFSET]) |
                  (static_cast<uint16_t>(data[ProtocolConstants::CRC16_OFFSET + 1]) << 8);

    // 解析消息类型 (1字节)
    frame.message_type = data[ProtocolConstants::MSG_TYPE_OFFSET];

    // 解析消息ID (2字节)
    frame.message_id = static_cast<uint16_t>(data[ProtocolConstants::MSG_ID_OFFSET]) |
                       (static_cast<uint16_t>(data[ProtocolConstants::MSG_ID_OFFSET + 1]) << 8);

    // 解析子消息ID (1字节)
    frame.sub_message_id = data[ProtocolConstants::SUB_MSG_ID_OFFSET];

    // 解析序列号 (2字节)
    frame.sequence = static_cast<uint16_t>(data[ProtocolConstants::SEQUENCE_OFFSET]) |
                     (static_cast<uint16_t>(data[ProtocolConstants::SEQUENCE_OFFSET + 1]) << 8);

    // 解析长度 (2字节)
    frame.length = static_cast<uint16_t>(data[ProtocolConstants::LENGTH_OFFSET]) |
                   (static_cast<uint16_t>(data[ProtocolConstants::LENGTH_OFFSET + 1]) << 8);

    // 解析负载
    if (data.size() >= ProtocolConstants::HEADER_SIZE + frame.length) {
      frame.payload.assign(data.begin() + ProtocolConstants::PAYLOAD_OFFSET,
                           data.begin() + ProtocolConstants::PAYLOAD_OFFSET + frame.length);
    }
  }

  return frame;
}

std::vector<uint8_t> ProtocolUtils::BuildFrame(const ProtocolFrame &frame) {
  std::vector<uint8_t> data;

  // 添加魔术字 (2字节)
  data.push_back(static_cast<uint8_t>(frame.magic_id & 0xFF));
  data.push_back(static_cast<uint8_t>((frame.magic_id >> 8) & 0xFF));

  // 预留CRC位置 (2字节)
  data.push_back(0);
  data.push_back(0);

  // 添加消息类型 (1字节)
  data.push_back(frame.message_type);

  // 添加消息ID (2字节)
  data.push_back(static_cast<uint8_t>(frame.message_id & 0xFF));
  data.push_back(static_cast<uint8_t>((frame.message_id >> 8) & 0xFF));

  // 添加子消息ID (1字节)
  data.push_back(frame.sub_message_id);

  // 添加序列号 (2字节)
  data.push_back(static_cast<uint8_t>(frame.sequence & 0xFF));
  data.push_back(static_cast<uint8_t>((frame.sequence >> 8) & 0xFF));

  // 添加长度 (2字节)
  data.push_back(static_cast<uint8_t>(frame.length & 0xFF));
  data.push_back(static_cast<uint8_t>((frame.length >> 8) & 0xFF));

  // 添加负载
  data.insert(data.end(), frame.payload.begin(), frame.payload.end());

  // 计算并设置CRC (位置在字节2-3)
  uint16_t crc = CalculateCRC16(data.data() + 4, data.size() - 4);
  data[ProtocolConstants::CRC16_OFFSET] = static_cast<uint8_t>(crc & 0xFF);
  data[ProtocolConstants::CRC16_OFFSET + 1] = static_cast<uint8_t>((crc >> 8) & 0xFF);

  return data;
}

bool ProtocolUtils::ValidateMessage(const std::vector<uint8_t> &data) {
  if (data.size() < ProtocolConstants::MIN_FRAME_SIZE) {
    return false;
  }

  // 检查魔数 (2字节)
  uint16_t magic_id = static_cast<uint16_t>(data[ProtocolConstants::MAGIC_OFFSET]) |
                      (static_cast<uint16_t>(data[ProtocolConstants::MAGIC_OFFSET + 1]) << 8);

  if (magic_id != ProtocolConstants::MAGIC_ID) {
    return false;
  }

  // 检查CRC (位置在字节2-3)
  uint16_t stored_crc = static_cast<uint16_t>(data[ProtocolConstants::CRC16_OFFSET]) |
                        (static_cast<uint16_t>(data[ProtocolConstants::CRC16_OFFSET + 1]) << 8);
  uint16_t calculated_crc = CalculateCRC16(data.data() + 4, data.size() - 4);

  return stored_crc == calculated_crc;
}

}  // namespace perception
