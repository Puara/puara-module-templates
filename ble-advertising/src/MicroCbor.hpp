/*********************************************************************************
 * SPDX-License-Identifier: MIT
 *
 * @brief Provide a minimal implementation of CBOR suitable for embedded use.
 *
 * Basic usage:
 *
 *  int32_t i32 = 12345678;
 *  int32_t *vec[] = {1,2,3,4};
 *
 *  // encode
 *  uint8_t   buf[200];
 *  MicroCbor cbor(buf, sizeof(buf));
 *  cbor.startMap();
 *  {
 *      // Add values using type inference to store properly
 *      cbor.add("i32", i32);
 *      cbor.add("vec", vec);
 *  }
 *  cbor.endMap();
 *
 * std::cout << "Bytes serialized " << cbor.bytesSerialized() << std::endl;
 *
 *  // decode a buffer
 *  MicroCbor cbor(buf, sizeof(buf));
 *  // query for value, use default if not found or incompatible
 *  auto i32 = cbor.get<int32_t>("i32",-1);
 *  const int32_t* vecinfo = cbor.getPointer<int32_t>("vec",nullptr);
 *  if (vecinfo.p != nullptr) {
 *     processData(vecinfo.p, vecinfo.length, i32);
 *  }
 *
 ********************************************************************************/
#pragma once

#include <string.h>  // strlen

#include <cstdint>
#include <cstring>      // memcpy
#include <type_traits>  // std::enable_if

#ifdef CONFIG_MICROCBOR_STD_VECTOR
#include <vector>
#endif

#ifndef CONFIG_MICROCBOR_MAX_NESTING
#define CONFIG_MICROCBOR_MAX_NESTING 4
#endif

#ifndef MicroCborSerializer
#define MicroCborSerializer MicroCborSerializer
#endif

namespace entazza {

// Encoding constants
constexpr uint8_t kCborPosInt = 0;
constexpr uint8_t kCborNegInt = 1;
constexpr uint8_t kCborByteString = 2;
constexpr uint8_t kCborUTF8String = 3;
constexpr uint8_t kCborArray = 4;
constexpr uint8_t kCborMap = 5;
constexpr uint8_t kCborTag = 6;
constexpr uint8_t kCborSimple = 7;
constexpr uint8_t kCborError = 8;
constexpr uint8_t kCborFalse = kCborSimple << 5 | 20;
constexpr uint8_t kCborTrue = kCborSimple << 5 | 21;
constexpr uint8_t kCborNull = kCborSimple << 5 | 22;
constexpr uint8_t kCborFloat32 = kCborSimple << 5 | 26;
constexpr uint8_t kCborFloat64 = kCborSimple << 5 | 27;

constexpr uint16_t kCborTagInvalid = 65535;
constexpr uint8_t kCborTagHomogeneousArray = 41;
constexpr uint8_t kCborTagUint8 = 64;
constexpr uint8_t kCborTagUint16 = 69;
constexpr uint8_t kCborTagUint32 = 70;
constexpr uint8_t kCborTagUint64 = 71;
constexpr uint8_t kCborTagInt8 = 72;
constexpr uint8_t kCborTagInt16 = 77;
constexpr uint8_t kCborTagInt32 = 78;
constexpr uint8_t kCborTagInt64 = 79;
constexpr uint8_t kCborTagFloat32 = 85;
constexpr uint8_t kCborTagFloat64 = 86;
constexpr uint16_t kCborTagTimeExt = 1001;
constexpr uint16_t kCborTagDurationExt = 1002;

/**
 * @brief Helpers to get a CBOR tag type given a template type
 *
 * Usage kCBorTagInfo<T>::tag will return a const tag value for type T
 *
 * @tparam T
 * @tparam void
 */
template <typename T, typename Dummy = void>
struct kCborTagInfo;
template <>
struct kCborTagInfo<int8_t> {
  constexpr static const uint8_t tag = kCborTagInt8;
};
template <>
struct kCborTagInfo<int16_t> {
  constexpr static const uint8_t tag = kCborTagInt16;
};
template <>
struct kCborTagInfo<int32_t> {
  constexpr static const uint8_t tag = kCborTagInt32;
};
template <>
struct kCborTagInfo<int64_t> {
  constexpr static const uint8_t tag = kCborTagInt64;
};
template <>
struct kCborTagInfo<uint8_t> {
  constexpr static const uint8_t tag = kCborTagUint8;
};
template <>
struct kCborTagInfo<uint16_t> {
  constexpr static const uint8_t tag = kCborTagUint16;
};
template <>
struct kCborTagInfo<uint32_t> {
  constexpr static const uint8_t tag = kCborTagUint32;
};
template <>
struct kCborTagInfo<uint64_t> {
  constexpr static const uint8_t tag = kCborTagUint64;
};
template <>
struct kCborTagInfo<float> {
  constexpr static const uint8_t tag = kCborTagFloat32;
};
template <>
struct kCborTagInfo<double> {
  constexpr static const uint8_t tag = kCborTagFloat64;
};

/**
 * @brief A class to encode and decode data in CBOR format.
 */
class MicroCbor {
  friend class MicroCborSerializer;

 private:
  struct TypeInfo {
    uint16_t tag;
    uint8_t majorval;
    uint8_t minorval;
    uint8_t headerBytes;
    uint8_t *p;
    TypeInfo(uint8_t majorval) : majorval(majorval) {}
    TypeInfo(uint16_t tag, uint8_t majorval, uint8_t minorval, uint8_t headerBytes,
             uint8_t *p)
        : tag(tag),
          majorval(majorval),
          minorval(minorval),
          headerBytes(headerBytes),
          p(p) {}
  };

  typedef struct {
    uint32_t mapStartPos;
    uint32_t mapStartCount;
    uint16_t mapCount;
  } MapState;

  uint8_t *mBuf;
  uint32_t mMaxBufLen;
  uint32_t mBufBytesNeeded;
  uint32_t mDataOffset;
  typedef int Error;
  Error mResult = 0;
  bool mReadOnly = false;
  bool mNullTerminate = false;  // True to null terminate user strings

  int8_t mDepth;  //< How deep we've nested maps
  MapState mMapState[CONFIG_MICROCBOR_MAX_NESTING];

  /**
   * @brief Reserve n bytes in the output buffer.
   * If n bytes are not available, an error code is set but
   * the total number of bytes needed is incremented for
   * later retrieval in case more bytes are needed.
   *
   * @param n The number of bytes needed.
   */
  inline void reserveBytes(const uint32_t n) noexcept {
    mBufBytesNeeded += n;
    if (mBufBytesNeeded > mMaxBufLen) {
      mResult = -1;
    }
  }

  /**
   * @brief Compute the number of tag bytes needed to encode a length value.
   *
   * @param length
   * @return uint8_t The number of bytes needed
   */
  inline uint8_t bytesForLength(const uint32_t length) {
    return (length < 24) ? 1 : (length < 256) ? 2 : (length < 0x10000) ? 3 : 4;
  }

  /**
   * @brief Get the Length value from the current tag
   *
   * @return uint32_t
   */
  uint32_t getLength() noexcept {
    uint32_t len = mBuf[mDataOffset] & 0x1f;
    auto p = mBuf + mDataOffset + 1;
    if (len < 24) {
      mDataOffset++;
      return len;
    }
    if (len == 24) {
      mDataOffset += 2;
      return *p;
    }
    if (len == 25) {
      mDataOffset += 3;
      return p[0] << 8 | p[1];
    }
    if (len == 26) {
      mDataOffset += 5;
      return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    }
    mResult = -1;
    return 0;  // unsupported;
  }

  /**
   * @brief Retrieve info about the next field in a map
   *
   * @return TypeInfo
   */
  TypeInfo getNextField() {
    static const uint8_t kCborheaderBytes[24 + 4]{1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                  1, 1, 1, 1, 2, 3, 5, 9};

    if (mDataOffset >= mMaxBufLen) {
      return TypeInfo(kCborError);
    }
    uint8_t *p = mBuf + mDataOffset;
    uint8_t majorval = *p >> 5;
    uint8_t minorval = *p & 0x1f;
    uint8_t headerBytes = kCborheaderBytes[minorval];
    if (mDataOffset + headerBytes > mMaxBufLen) {
      return TypeInfo(kCborError);
    }
    TypeInfo field = TypeInfo(kCborTagInvalid, majorval, minorval, headerBytes, p);

    if (majorval == kCborTag) {
      // next field is the actual 'value'
      auto tag = getFieldValue(field);
      skipField(field);
      field = getNextField();
      field.tag = tag;
    }
    return field;
  }
  template <typename T = uint32_t>
  inline T getFieldValue(const TypeInfo &info) {
    uint8_t *p = info.p + 1;
    switch (info.headerBytes) {
      case 1:
        return info.minorval;
      case 2:
        return *p;
      case 3:
        return uint16_t(p[0]) << 8 | p[1];
      case 5:
        return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 |
               uint32_t(p[2]) << 8 | p[3];
      case 9:
        return uint64_t(p[0]) << 56 | uint64_t(p[1]) << 48 |
               uint64_t(p[2]) << 40 | uint64_t(p[3]) << 32 |
               uint64_t(p[4]) << 24 | uint64_t(p[5]) << 16 |
               uint64_t(p[6]) << 8 | uint64_t(p[7]);
      default:
        return 0;
    }
  }

  /**
   * @brief Skip over a field in a map
   *
   * @param info
   */
  void skipField(const TypeInfo &info) noexcept {
    auto len = getFieldValue(info);
    mDataOffset += info.headerBytes;
    if (mDataOffset >= mMaxBufLen) {
      return;
    }

    switch (info.majorval) {
      case kCborByteString:
      case kCborUTF8String: {
        mDataOffset += len;
        break;
      }
      case kCborMap: {
        while (len--) {
          // Skip key/value pair
          auto key = getNextField();
          skipField(key);
          auto value = getNextField();
          skipField(value);
        }
        break;
      }
      case kCborArray:
        while (len--) {
          auto field = getNextField();
          skipField(field);
        }
      default:
        break;
    }
  }

  /**
   * @brief Find the named element in a map.
   *
   * If there are no more fields, the major value is
   * set to kCborError;
   *
   * If a match is found the return value reflects information about the field
   * after the name.
   *
   * @param name
   * @return TypeInfo
   */
  TypeInfo findElement(const char *name) noexcept {
    auto mapOffset = mDataOffset;
    auto info = getNextField();
    // We must be in a map to find anything
    if (info.majorval != kCborMap) {
      return TypeInfo(kCborError);
    }

    auto len = strlen(name);
    auto numItems = getFieldValue(info);
    mDataOffset += info.headerBytes;  // skip map length
    while (numItems-- != 0) {
      auto s = getNextField();
      auto sLen = getFieldValue(s);
      const auto key = (const char *)mBuf + mDataOffset + info.headerBytes;
      if (len <= sLen && strncmp(name, key, sLen) == 0) {
        skipField(s);  // skip over name
        auto value = getNextField();
        mDataOffset = mapOffset;
        return value;
      }
      skipField(s);
      auto value = getNextField();
      skipField(value);
    }

    // restore offset to beginning of map
    mDataOffset = mapOffset;
    return TypeInfo(kCborError);
  }

  /**
   * @brief Store a byte into the output buffer, incrementing the output
   * position.
   *
   * @param v The byte to store.
   */
  inline void storeByte(const uint8_t v) noexcept {
    if (mResult) {
      return;
    }
    mBuf[mDataOffset++] = v;
  }

  inline void encodeUInt8(const uint8_t tag, const uint8_t value) {
    reserveBytes(2);
    if (mResult == 0) {
      uint8_t *b = mBuf + mDataOffset;
      b[0] = tag;
      b[1] = value;
      mDataOffset += 2;
    }
  }
  inline void encodeUInt16(const uint8_t tag, const uint16_t value) {
    reserveBytes(3);
    if (mResult == 0) {
      uint8_t *b = mBuf + mDataOffset;
      *b++ = tag;
      *b++ = uint8_t(value >> 8);
      *b = uint8_t(value);
      mDataOffset += 3;
    }
  }
  inline void encodeUInt32(const uint8_t tag, const uint32_t value) {
    reserveBytes(5);
    if (mResult == 0) {
      uint8_t *b = mBuf + mDataOffset;
      // printk("val=%08x\n",value);

      *b++ = tag;
      *b++ = value >> 24;
      *b++ = value >> 16;
      *b++ = value >> 8;
      *b++ = uint8_t(value);
      mDataOffset += 5;
    }
  }

  /**
   * @brief Encode a 64 bit value using the provided tag.
   *
   * @param tag
   * @param value
   */
  inline void encodeUInt64(const uint8_t tag, const uint64_t value) {
    reserveBytes(9);
    if (mResult == 0) {
      uint8_t *b = mBuf + mDataOffset;
      *b++ = tag;
      *b++ = value >> 56;
      *b++ = value >> 48;
      *b++ = value >> 40;
      *b++ = value >> 32;
      *b++ = value >> 24;
      *b++ = value >> 16;
      *b++ = value >> 8;
      *b++ = uint8_t(value);
      mDataOffset += 9;
    }
  }

  /**
   * @brief Encode an unsigned integer length field along with a CBOR major
   * type.
   *
   * @param majorval
   * @param len
   */
  void encodeHeader(const uint8_t majorval, const uint32_t len) noexcept {
    if (len < 24) {
      reserveBytes(1);
      storeByte(majorval << 5 | len);
    } else if (len < 256) {
      reserveBytes(2);
      storeByte(majorval << 5 | 24);
      storeByte(len);
    } else if (len < 65536) {
      encodeUInt16(majorval << 5 | 25, uint16_t(len));
    } else {
      encodeUInt32(majorval << 5 | 26, len);
    }
  }

  /**
   * @brief Encode a type tag.
   * Type tags can be 1-3 bytes.  This implementation only
   * uses 1 or 2 byte tag values.
   *
   * @param tag
   */
  inline void encodeTag(const uint16_t tag) noexcept {
    if (tag < 24) {
      reserveBytes(1);
      storeByte(kCborTag << 5 | tag);
    } else if (tag < 256) {
      reserveBytes(2);
      storeByte(kCborTag << 5 | 24);
      storeByte(tag);
    } else {
      reserveBytes(3);
      storeByte(kCborTag << 5 | 25);
      storeByte(tag >> 8);
      storeByte(tag);
    }
  }

  /**
   * @brief Encode a string into the output buffer.
   *
   * If nullTerminate is true an extra byte is serialized to allow null
   * terminated strings to be used 'inplace' when decoding.
   *
   * @param value The string to encode.
   * @param nullTerminate true to add null termination to the output stream.
   */
  void encodeString(const char *value,
                    const bool nullTerminate = false) noexcept {
    auto len = strlen(value);
    if (nullTerminate) {
      len++;
    }
    encodeHeader(kCborUTF8String, len);
    reserveBytes(len);

    if (mResult == 0) {
      memcpy(mBuf + mDataOffset, value, len);
      mDataOffset += len;
    }
  }

  inline void encodeMapKey(const char *value) {
    if (value == nullptr || *value == 0) {
      return;  // ignore.  Used for 'List' encoding
    }
    mMapState[mDepth].mapCount++;
    encodeString(value);
  }

  /**
   * @brief Encode a sequency of bytes into the output buffer
   *
   * @param bytes A pointer to the bytes to transfer to the output buffer
   * @param numBytes The number of bytes to encode
   */
  inline void encodeBytes(const void *bytes, const uint32_t numBytes) noexcept {
    encodeHeader(kCborByteString, numBytes);

    reserveBytes(numBytes);
    if (mResult == 0) {
      memcpy(mBuf + mDataOffset, bytes, numBytes);
      mDataOffset += numBytes;
    }
  }

 public:
  MicroCbor() { this->initBuffer((void *)0, 0); }
  /**
   * @brief Construct a new Micro Cbor object
   *
   * @param buf A pointer to a working i/o buffer
   * @param maxBufLen The length in bytes of the buffer
   * @param nullTermiante True to null terminate user strings when serializing
   * to assist with in-place reads
   */
  MicroCbor(void *buf, const uint32_t maxBufLen,
            const bool nullTerminate = true)
      : mNullTerminate(nullTerminate) {
    this->initBuffer(buf, maxBufLen);
  }

  /**
   * @brief Construct a new Micro Cbor object with a read-only buffer.
   *
   * This instance can only be used for decoding.
   *
   * @param buf A pointer to a working i/o buffer
   * @param maxBufLen The length in bytes of the buffer
   * @param nullTermiante True to null terminate user strings to assist with
   * in-place reads
   */
  MicroCbor(const void *buf, const uint32_t maxBufLen,
            const bool nullTerminate = false)
      : mNullTerminate(nullTerminate) {
    this->initBuffer(buf, maxBufLen);
  }

  /**
   * @brief Reinitialize the working buffer.
   *
   * @param buf A pointer to a buffer
   * @param maxBufLen The length in bytes of the buffer
   */
  inline void initBuffer(void *buf, const uint32_t maxBufLen) noexcept {
    this->mBuf = (uint8_t *)buf;
    this->mMaxBufLen = maxBufLen;
    this->mDepth = -1;
    this->mResult = 0;
    this->mDataOffset = 0;
    this->mBufBytesNeeded = 0;
    this->mReadOnly = false;
  }

  /**
   * @brief Reinitialize the working buffer with a read-only buffer.
   *
   * This buffer can only be used for decoding cbor streams.
   *
   * @param buf A pointer to a buffer
   * @param maxBufLen The length in bytes of the buffer
   */
  inline void initBuffer(const void *buf, const uint32_t maxBufLen) noexcept {
    initBuffer(const_cast<void *>(buf), maxBufLen);
    this->mReadOnly = true;
  }

  /**
   * @brief Reset the encoder/decoder state to allow using again
   *
   */
  inline void restart() noexcept {
    this->mDepth = -1;
    this->mResult = 0;
    this->mDataOffset = 0;
    this->mBufBytesNeeded = 0;
  }

  /**
   * @brief Get the result of encoding.
   * If non-zero the output buffer was not large enough.  In
   * this case use the bytesNeeded() method to query how big
   * the buffer needs to be.
   *
   * @return Error
   */
  inline Error getResult() const noexcept { return mResult; }

  /**
   * @brief Get a pointer to the internal output buffer.
   *
   * @return const uint8_t*
   */
  inline const uint8_t *getBuffer() { return mBuf; }

  /**
   * @brief Get the total number of bytes serialized.
   *
   * @return uint32_t
   */
  inline uint32_t bytesSerialized() const noexcept { return mDataOffset; }

  /**
   * @brief Get the total number of bytes needed to encode the supplied fields.
   *
   * This number can be larger than bytesSerialized() if the buffer was not
   * large enough to hold the complete serialization.
   *
   * @return uint32_t
   */
  inline uint32_t bytesNeeded() const noexcept { return mBufBytesNeeded; }

  /**
   * @brief Start a map with the indicated number of
   * map key/value pairs.  This is a hint to the maximum
   * number of key/value pairs.  If 0 is used it is
   * assumed that no more than 255 fields will be present.
   *
   * @param numElements
   * @return Error
   */
  Error startMap(const uint32_t numElements = 0) noexcept {
    if (mReadOnly || mDepth >= CONFIG_MICROCBOR_MAX_NESTING) {
      mResult = -1;
      return mResult;
    }
    mDepth += 1;
    mMapState[mDepth].mapStartPos = mDataOffset;
    mMapState[mDepth].mapStartCount = numElements;
    mMapState[mDepth].mapCount = 0;
    encodeHeader(kCborMap, numElements);
    return mResult;
  }

  /**
   * @brief Complete map encoding
   * If the number of fields is different than that provided to
   * the startMap function the serialized data is updated with
   * the actual number of fields encoded.
   *
   * @return Error
   */
  inline Error endMap() noexcept {
    MapState &map = mMapState[mDepth];
    // update map count
    if (mResult == 0 && map.mapCount != map.mapStartCount) {
      if (map.mapCount < 24) {
        mBuf[map.mapStartPos] = kCborMap << 5 | map.mapCount;
      } else {
        mBuf[map.mapStartPos] = kCborMap << 5 | 24;
        mBuf[map.mapStartPos + 1] = uint8_t(map.mapCount);
      }
    }

    mDepth -= 1;
    return mResult;
  }

  Error startMap(const char *name, uint8_t numElements = 0) {
    encodeMapKey(name);
    startMap();
    return mResult;
  }
  /**
   * @brief Add an unsigned or signed integer value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  template <typename T = uint32_t,
            typename std::enable_if<(!std::is_same<bool, T>::value)>::type * =
                nullptr>
  Error add(const char *name, const T value) noexcept {
    T intValue = value;
    encodeMapKey(name);
    uint8_t tag = kCborPosInt << 5;
    if (value < 0) {
      tag = kCborNegInt << 5;
      intValue = -1 - intValue;
    }

    if (sizeof(T) == 8) {
      encodeUInt64(tag | 27, intValue);
    } else if (sizeof(T) == 4) {
      encodeUInt32(tag | 26, intValue);
    } else if (sizeof(T) == 2) {
      encodeUInt16(tag | 25, intValue);
    } else {
      encodeUInt8(tag | 24, intValue);
    }

    return mResult;
  }

  /**
   * @brief Add a boolean value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  Error add(const char *name, const bool value) noexcept {
    encodeMapKey(name);
    reserveBytes(1);
    storeByte(value ? kCborTrue : kCborFalse);
    return mResult;
  }

  /**
   * @brief Add a string value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  Error add(const char *name, const char *value) noexcept {
    encodeMapKey(name);
    encodeString(value, mNullTerminate);
    return mResult;
  }

  /**
   * @brief Add a string value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  Error add(const char *name, char *value) noexcept {
    encodeMapKey(name);
    encodeString(value, mNullTerminate);
    return mResult;
  }

  /**
   * @brief Add a unsigned or signed integer value to the output buffer
   *  The value is stored to reduce storage when possible.
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  template <typename T = uint32_t,
            typename std::enable_if<(!std::is_same<bool, T>::value)>::type * =
                nullptr>
  Error addMinimal(const char *name, const T value) noexcept {
    encodeMapKey(name);
    if (value >= 0) {
      encodeHeader(kCborPosInt, value);
    } else {
      encodeHeader(kCborNegInt, -1 - value);
    }
    return mResult;
  }

  /**
   * @brief Add a float32 value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  Error add(const char *name, const float value) noexcept {
    const void *p = &value;

    encodeMapKey(name);
    encodeUInt32(kCborFloat32, *(uint32_t *)p);
    return mResult;
  }

  /**
   * @brief Add a float64 value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  Error add(const char *name, const double value) noexcept {
    const void *p = &value;
    encodeMapKey(name);
    encodeUInt64(kCborFloat64, *(uint64_t *)p);
    return mResult;
  }

  /**
   * @brief Add an array of data to the output buffer
   *
   * @param name The key name to associate with the value.  Omit if null.
   * @param value The value to store
   * @return Error
   */
  template <typename T>
  Error add(const char *name, const T *value, const uint32_t numElements,
            const bool align = true) {
    const auto numRawBytes = numElements * sizeof(T);
    if (name != nullptr && align) {
      // compute the length of the name header to get offset for vector data
      // If padding is needed, inject nulls after the key name string
      auto len = strlen(name);
      auto preambleBytes =
          len + bytesForLength(len) + 2 /*tag*/ + bytesForLength(numRawBytes);
      auto vectorOffset = mBufBytesNeeded + preambleBytes;
      auto alignBytes = sizeof(T);
      auto oddBytes = vectorOffset % alignBytes;

      if (oddBytes == 0) {
        encodeMapKey(name);
      } else {
        auto paddingNeeded = alignBytes - oddBytes;
        // Add key/value pair
        encodeHeader(kCborUTF8String, len + paddingNeeded);
        reserveBytes(len + paddingNeeded);
        if (mResult == 0) {
          memcpy(mBuf + mDataOffset, name, len);
          memset(mBuf + mDataOffset + len, 0, paddingNeeded);
          mDataOffset += len + paddingNeeded;
        }
      }
    } else {
      encodeMapKey(name);
    }
    encodeTag(kCborTagInfo<T>::tag);
    encodeBytes(value, numRawBytes);
    return mResult;
  }

#ifdef CONFIG_MICROCBOR_STD_VECTOR
  /**
   * @brief Add a std::vector<numeric> value to the output buffer
   *
   * @param name The key name to associate with the value
   * @param value The value to store
   * @return Error
   */
  template <typename T>
  inline Error add(const char *name, const std::vector<T> &value,
                   const bool align = true) noexcept {
    return add(name, value.data(), value.size(), align);
  }
#endif

  /**
   * @brief Get a map element with the specified key name.
   * If the key name is not present or is not a map an empty MicroCbor instance
   * is returned.
   *
   * @param name The key name to look up.
   * @return A MicroCbor instance suitable for reading values from the map.
   */
  MicroCbor getMap(const char *name) {
    auto element = findElement(name);
    if (element.majorval == kCborMap) {
      return MicroCbor(element.p, mMaxBufLen - mDataOffset);
    } else {
      return MicroCbor();
    }
  }

  /**
   * @brief Get an unsigned or signed integer value with the specified key name.
   * If the value is not present, the default value is returned.
   *
   * @param name The key name to look up.
   * @return The value in the map or the defaultValue.
   */
  template <typename T,
            typename std::enable_if<
                (std::is_integral<T>::value && !std::is_same<bool, T>::value &&
                 !std::is_same<float, T>::value)>::type * = nullptr>
  T get(const char *name, const T defaultValue) noexcept {
    auto element = findElement(name);
    if (element.headerBytes == 9) {
      uint8_t *p = element.p + 1;
      uint64_t value = uint64_t(p[0]) << 56 | uint64_t(p[1]) << 48 |
                       uint64_t(p[2]) << 40 | uint64_t(p[3]) << 32 |
                       uint64_t(p[4]) << 24 | uint64_t(p[5]) << 16 |
                       uint64_t(p[6]) << 8 | uint64_t(p[7]);
      if (element.majorval == kCborPosInt) {
        return T(value);
      }
      if (element.majorval == kCborNegInt) {
        return T(-value - 1);
      }
    } else {
      auto value = getFieldValue(element);
      if (element.majorval == kCborPosInt) {
        return T(value);
      }
      if (element.majorval == kCborNegInt) {
        return T(-value - 1);
      }
    }
    return defaultValue;
  }

  /**
   * @brief Get a boolean value with the specified key name.  If the value is
   * not present, the default value is returned.
   *
   * @param name The key name to look up.
   * @return The value in the map or the defaultValue.
   * @return bool
   */
  template <typename T, typename std::enable_if<
                            (std::is_same<bool, T>::value)>::type * = nullptr>
  T get(const char *name, const T defaultValue) noexcept {
    auto element = findElement(name);

    if (element.majorval == kCborSimple) {
      if (element.minorval == 20) {
        return false;
      } else if (element.minorval == 21) {
        return true;
      } else {
        return defaultValue;
      }
    }

    return defaultValue;
  }

  /**
   * @brief Get a float value with the specified key name.  If the value is
   * not present, the default value is returned.
   *
   * @param name The key name to look up.
   * @return The value in the map or the defaultValue.
   * @return float
   */
  template <typename T, typename std::enable_if<
                            (std::is_same<float, T>::value)>::type * = nullptr>
  T get(const char *name, const T defaultValue) noexcept {
    auto element = findElement(name);

    if (element.majorval == kCborSimple) {
      if (element.minorval == 26) {
        uint32_t f = getFieldValue(element);
        return *(float *)&f;
      } else {
        return defaultValue;
      }
    }

    return defaultValue;
  }

  /**
   * @brief Get a string value with the specified key name.  If the value is
   * not present, the default value is returned.
   *
   * This method returns a pointer to the beginning of the string and assumes
   * that this encoder has been used to encode a string with a null termination
   * for convenience.
   *
   * @param name The key name to look up.
   * @return The value in the map or the defaultValue.
   * @return Pointer to string
   */
  template <typename T, typename std::enable_if<
                            (std::is_same<const char *, T>::value ||
                             std::is_same<char *, T>::value)>::type * = nullptr>
  const char *get(const char *name, T defaultValue) noexcept {
    auto element = findElement(name);
    if (element.majorval == kCborUTF8String) {
      const char *s = (const char *)(element.p + element.headerBytes);
      return s;
    }

    return defaultValue;
  }

  /**
   * @brief Get the length of an item.
   *
   * Depending on the item, the length can have different meanings so the use of
   * this function is 'user beware'.
   *
   * For arrays this returns the number of bytes.
   * For strings it returns the length of the string not including a null
   * termination. For maps it returns the number of items in the map
   *
   * If the field is not found, zero is returned.
   *
   * @param name The name of the field to find
   * @return uint32_t
   */
  uint32_t getLength(const char *name) noexcept {
    auto element = findElement(name);
    if (element.majorval != kCborError) {
      auto len = getFieldValue(element);
      if (element.majorval == kCborUTF8String &&
          element.p[element.headerBytes + len - 1] == 0) {
        // do not count the attached null bytes
        len -= 1;
      }
      return len;
    } else {
      return 0;
    }
  }

  template <typename T>
  struct CborArray {
    size_t length;
    const T *p;
  };
  /**
   * @brief Get a pointer to vector data.
   *
   * If the named parameter is not present, the defaultValue is returned
   *
   * @tparam T The type of vector data expected.
   * @param name The name of the field to find
   * @param defaultValue The value to return if the name is not present or an
   * error occurs
   * @return struct CborArray with length an pointer to data
   */
  template <typename T>
  struct CborArray<T> getPointer(const char *name,
                                 const T *defaultValue) noexcept {
    auto element = findElement(name);
    if (element.tag != kCborTagInfo<T>::tag) {
      return {.length = 0, .p = defaultValue};
    }
    auto length = getFieldValue(element) / sizeof(T);
    const T *p = (T *)(element.p + element.headerBytes);

    return {.length = length, .p = p};
  }
};
static_assert(sizeof(double) == 8, "Unexpected `double` size");

}  // namespace entazza
