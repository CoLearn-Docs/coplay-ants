// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_LOCALMSG_COPLAY_LOCALMESSAGE_H_
#define FLATBUFFERS_GENERATED_LOCALMSG_COPLAY_LOCALMESSAGE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 3,
             "Non-compatible flatbuffers version included");

namespace CoPlay {
namespace LocalMessage {

struct LocalMessage;
struct LocalMessageBuilder;

struct RegistrationData;
struct RegistrationDataBuilder;

struct ControlData;
struct ControlDataBuilder;

struct BinaryData;
struct BinaryDataBuilder;

enum Data : uint8_t {
  Data_NONE = 0,
  Data_RegistrationData = 1,
  Data_ControlData = 2,
  Data_BinaryData = 3,
  Data_MIN = Data_NONE,
  Data_MAX = Data_BinaryData
};

inline const Data (&EnumValuesData())[4] {
  static const Data values[] = {
    Data_NONE,
    Data_RegistrationData,
    Data_ControlData,
    Data_BinaryData
  };
  return values;
}

inline const char * const *EnumNamesData() {
  static const char * const names[5] = {
    "NONE",
    "RegistrationData",
    "ControlData",
    "BinaryData",
    nullptr
  };
  return names;
}

inline const char *EnumNameData(Data e) {
  if (::flatbuffers::IsOutRange(e, Data_NONE, Data_BinaryData)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesData()[index];
}

template<typename T> struct DataTraits {
  static const Data enum_value = Data_NONE;
};

template<> struct DataTraits<CoPlay::LocalMessage::RegistrationData> {
  static const Data enum_value = Data_RegistrationData;
};

template<> struct DataTraits<CoPlay::LocalMessage::ControlData> {
  static const Data enum_value = Data_ControlData;
};

template<> struct DataTraits<CoPlay::LocalMessage::BinaryData> {
  static const Data enum_value = Data_BinaryData;
};

bool VerifyData(::flatbuffers::Verifier &verifier, const void *obj, Data type);
bool VerifyDataVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types);

struct LocalMessage FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef LocalMessageBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TRACK = 4,
    VT_DATA_TYPE = 6,
    VT_DATA = 8
  };
  const ::flatbuffers::String *track() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TRACK);
  }
  CoPlay::LocalMessage::Data data_type() const {
    return static_cast<CoPlay::LocalMessage::Data>(GetField<uint8_t>(VT_DATA_TYPE, 0));
  }
  const void *data() const {
    return GetPointer<const void *>(VT_DATA);
  }
  template<typename T> const T *data_as() const;
  const CoPlay::LocalMessage::RegistrationData *data_as_RegistrationData() const {
    return data_type() == CoPlay::LocalMessage::Data_RegistrationData ? static_cast<const CoPlay::LocalMessage::RegistrationData *>(data()) : nullptr;
  }
  const CoPlay::LocalMessage::ControlData *data_as_ControlData() const {
    return data_type() == CoPlay::LocalMessage::Data_ControlData ? static_cast<const CoPlay::LocalMessage::ControlData *>(data()) : nullptr;
  }
  const CoPlay::LocalMessage::BinaryData *data_as_BinaryData() const {
    return data_type() == CoPlay::LocalMessage::Data_BinaryData ? static_cast<const CoPlay::LocalMessage::BinaryData *>(data()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_TRACK) &&
           verifier.VerifyString(track()) &&
           VerifyField<uint8_t>(verifier, VT_DATA_TYPE, 1) &&
           VerifyOffset(verifier, VT_DATA) &&
           VerifyData(verifier, data(), data_type()) &&
           verifier.EndTable();
  }
};

template<> inline const CoPlay::LocalMessage::RegistrationData *LocalMessage::data_as<CoPlay::LocalMessage::RegistrationData>() const {
  return data_as_RegistrationData();
}

template<> inline const CoPlay::LocalMessage::ControlData *LocalMessage::data_as<CoPlay::LocalMessage::ControlData>() const {
  return data_as_ControlData();
}

template<> inline const CoPlay::LocalMessage::BinaryData *LocalMessage::data_as<CoPlay::LocalMessage::BinaryData>() const {
  return data_as_BinaryData();
}

struct LocalMessageBuilder {
  typedef LocalMessage Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_track(::flatbuffers::Offset<::flatbuffers::String> track) {
    fbb_.AddOffset(LocalMessage::VT_TRACK, track);
  }
  void add_data_type(CoPlay::LocalMessage::Data data_type) {
    fbb_.AddElement<uint8_t>(LocalMessage::VT_DATA_TYPE, static_cast<uint8_t>(data_type), 0);
  }
  void add_data(::flatbuffers::Offset<void> data) {
    fbb_.AddOffset(LocalMessage::VT_DATA, data);
  }
  explicit LocalMessageBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<LocalMessage> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<LocalMessage>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<LocalMessage> CreateLocalMessage(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> track = 0,
    CoPlay::LocalMessage::Data data_type = CoPlay::LocalMessage::Data_NONE,
    ::flatbuffers::Offset<void> data = 0) {
  LocalMessageBuilder builder_(_fbb);
  builder_.add_data(data);
  builder_.add_track(track);
  builder_.add_data_type(data_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<LocalMessage> CreateLocalMessageDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *track = nullptr,
    CoPlay::LocalMessage::Data data_type = CoPlay::LocalMessage::Data_NONE,
    ::flatbuffers::Offset<void> data = 0) {
  auto track__ = track ? _fbb.CreateString(track) : 0;
  return CoPlay::LocalMessage::CreateLocalMessage(
      _fbb,
      track__,
      data_type,
      data);
}

struct RegistrationData FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RegistrationDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SSID = 4,
    VT_PASSWORD = 6,
    VT_HOST = 8,
    VT_PORT = 10,
    VT_PATH = 12,
    VT_TRACK_CAMERA = 14,
    VT_TRACK_CONTROL = 16,
    VT_PROFILE = 18
  };
  const ::flatbuffers::String *ssid() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SSID);
  }
  const ::flatbuffers::String *password() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PASSWORD);
  }
  const ::flatbuffers::String *host() const {
    return GetPointer<const ::flatbuffers::String *>(VT_HOST);
  }
  int16_t port() const {
    return GetField<int16_t>(VT_PORT, 0);
  }
  const ::flatbuffers::String *path() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PATH);
  }
  const ::flatbuffers::String *track_camera() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TRACK_CAMERA);
  }
  const ::flatbuffers::String *track_control() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TRACK_CONTROL);
  }
  const ::flatbuffers::String *profile() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PROFILE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SSID) &&
           verifier.VerifyString(ssid()) &&
           VerifyOffset(verifier, VT_PASSWORD) &&
           verifier.VerifyString(password()) &&
           VerifyOffset(verifier, VT_HOST) &&
           verifier.VerifyString(host()) &&
           VerifyField<int16_t>(verifier, VT_PORT, 2) &&
           VerifyOffset(verifier, VT_PATH) &&
           verifier.VerifyString(path()) &&
           VerifyOffset(verifier, VT_TRACK_CAMERA) &&
           verifier.VerifyString(track_camera()) &&
           VerifyOffset(verifier, VT_TRACK_CONTROL) &&
           verifier.VerifyString(track_control()) &&
           VerifyOffset(verifier, VT_PROFILE) &&
           verifier.VerifyString(profile()) &&
           verifier.EndTable();
  }
};

struct RegistrationDataBuilder {
  typedef RegistrationData Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_ssid(::flatbuffers::Offset<::flatbuffers::String> ssid) {
    fbb_.AddOffset(RegistrationData::VT_SSID, ssid);
  }
  void add_password(::flatbuffers::Offset<::flatbuffers::String> password) {
    fbb_.AddOffset(RegistrationData::VT_PASSWORD, password);
  }
  void add_host(::flatbuffers::Offset<::flatbuffers::String> host) {
    fbb_.AddOffset(RegistrationData::VT_HOST, host);
  }
  void add_port(int16_t port) {
    fbb_.AddElement<int16_t>(RegistrationData::VT_PORT, port, 0);
  }
  void add_path(::flatbuffers::Offset<::flatbuffers::String> path) {
    fbb_.AddOffset(RegistrationData::VT_PATH, path);
  }
  void add_track_camera(::flatbuffers::Offset<::flatbuffers::String> track_camera) {
    fbb_.AddOffset(RegistrationData::VT_TRACK_CAMERA, track_camera);
  }
  void add_track_control(::flatbuffers::Offset<::flatbuffers::String> track_control) {
    fbb_.AddOffset(RegistrationData::VT_TRACK_CONTROL, track_control);
  }
  void add_profile(::flatbuffers::Offset<::flatbuffers::String> profile) {
    fbb_.AddOffset(RegistrationData::VT_PROFILE, profile);
  }
  explicit RegistrationDataBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RegistrationData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RegistrationData>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<RegistrationData> CreateRegistrationData(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> ssid = 0,
    ::flatbuffers::Offset<::flatbuffers::String> password = 0,
    ::flatbuffers::Offset<::flatbuffers::String> host = 0,
    int16_t port = 0,
    ::flatbuffers::Offset<::flatbuffers::String> path = 0,
    ::flatbuffers::Offset<::flatbuffers::String> track_camera = 0,
    ::flatbuffers::Offset<::flatbuffers::String> track_control = 0,
    ::flatbuffers::Offset<::flatbuffers::String> profile = 0) {
  RegistrationDataBuilder builder_(_fbb);
  builder_.add_profile(profile);
  builder_.add_track_control(track_control);
  builder_.add_track_camera(track_camera);
  builder_.add_path(path);
  builder_.add_host(host);
  builder_.add_password(password);
  builder_.add_ssid(ssid);
  builder_.add_port(port);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<RegistrationData> CreateRegistrationDataDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *ssid = nullptr,
    const char *password = nullptr,
    const char *host = nullptr,
    int16_t port = 0,
    const char *path = nullptr,
    const char *track_camera = nullptr,
    const char *track_control = nullptr,
    const char *profile = nullptr) {
  auto ssid__ = ssid ? _fbb.CreateString(ssid) : 0;
  auto password__ = password ? _fbb.CreateString(password) : 0;
  auto host__ = host ? _fbb.CreateString(host) : 0;
  auto path__ = path ? _fbb.CreateString(path) : 0;
  auto track_camera__ = track_camera ? _fbb.CreateString(track_camera) : 0;
  auto track_control__ = track_control ? _fbb.CreateString(track_control) : 0;
  auto profile__ = profile ? _fbb.CreateString(profile) : 0;
  return CoPlay::LocalMessage::CreateRegistrationData(
      _fbb,
      ssid__,
      password__,
      host__,
      port,
      path__,
      track_camera__,
      track_control__,
      profile__);
}

struct ControlData FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ControlDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DIRECTION = 4,
    VT_VALUE = 6
  };
  const ::flatbuffers::String *direction() const {
    return GetPointer<const ::flatbuffers::String *>(VT_DIRECTION);
  }
  int32_t value() const {
    return GetField<int32_t>(VT_VALUE, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_DIRECTION) &&
           verifier.VerifyString(direction()) &&
           VerifyField<int32_t>(verifier, VT_VALUE, 4) &&
           verifier.EndTable();
  }
};

struct ControlDataBuilder {
  typedef ControlData Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_direction(::flatbuffers::Offset<::flatbuffers::String> direction) {
    fbb_.AddOffset(ControlData::VT_DIRECTION, direction);
  }
  void add_value(int32_t value) {
    fbb_.AddElement<int32_t>(ControlData::VT_VALUE, value, 0);
  }
  explicit ControlDataBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ControlData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ControlData>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ControlData> CreateControlData(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> direction = 0,
    int32_t value = 0) {
  ControlDataBuilder builder_(_fbb);
  builder_.add_value(value);
  builder_.add_direction(direction);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ControlData> CreateControlDataDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *direction = nullptr,
    int32_t value = 0) {
  auto direction__ = direction ? _fbb.CreateString(direction) : 0;
  return CoPlay::LocalMessage::CreateControlData(
      _fbb,
      direction__,
      value);
}

struct BinaryData FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef BinaryDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DATA = 4
  };
  const ::flatbuffers::Vector<uint8_t> *data() const {
    return GetPointer<const ::flatbuffers::Vector<uint8_t> *>(VT_DATA);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_DATA) &&
           verifier.VerifyVector(data()) &&
           verifier.EndTable();
  }
};

struct BinaryDataBuilder {
  typedef BinaryData Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_data(::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> data) {
    fbb_.AddOffset(BinaryData::VT_DATA, data);
  }
  explicit BinaryDataBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<BinaryData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<BinaryData>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<BinaryData> CreateBinaryData(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> data = 0) {
  BinaryDataBuilder builder_(_fbb);
  builder_.add_data(data);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<BinaryData> CreateBinaryDataDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *data = nullptr) {
  auto data__ = data ? _fbb.CreateVector<uint8_t>(*data) : 0;
  return CoPlay::LocalMessage::CreateBinaryData(
      _fbb,
      data__);
}

inline bool VerifyData(::flatbuffers::Verifier &verifier, const void *obj, Data type) {
  switch (type) {
    case Data_NONE: {
      return true;
    }
    case Data_RegistrationData: {
      auto ptr = reinterpret_cast<const CoPlay::LocalMessage::RegistrationData *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Data_ControlData: {
      auto ptr = reinterpret_cast<const CoPlay::LocalMessage::ControlData *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Data_BinaryData: {
      auto ptr = reinterpret_cast<const CoPlay::LocalMessage::BinaryData *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyDataVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyData(
        verifier,  values->Get(i), types->GetEnum<Data>(i))) {
      return false;
    }
  }
  return true;
}

inline const CoPlay::LocalMessage::LocalMessage *GetLocalMessage(const void *buf) {
  return ::flatbuffers::GetRoot<CoPlay::LocalMessage::LocalMessage>(buf);
}

inline const CoPlay::LocalMessage::LocalMessage *GetSizePrefixedLocalMessage(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<CoPlay::LocalMessage::LocalMessage>(buf);
}

inline bool VerifyLocalMessageBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<CoPlay::LocalMessage::LocalMessage>(nullptr);
}

inline bool VerifySizePrefixedLocalMessageBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<CoPlay::LocalMessage::LocalMessage>(nullptr);
}

inline void FinishLocalMessageBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<CoPlay::LocalMessage::LocalMessage> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLocalMessageBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<CoPlay::LocalMessage::LocalMessage> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace LocalMessage
}  // namespace CoPlay

#endif  // FLATBUFFERS_GENERATED_LOCALMSG_COPLAY_LOCALMESSAGE_H_
