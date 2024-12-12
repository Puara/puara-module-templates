#pragma once
#include <mdns.h>
#include "model.hpp"
#include <OSCMessage.h>
#include <string_view>
#include <optional>

static inline float get_float(OSCMessage &msg)
{
  if (msg.isFloat(0))
    return msg.getFloat(0);
  else if (msg.isInt(0))
    return msg.getInt(0);
  else if (msg.isBoolean(0))
    return msg.getBoolean(0) ? 1.f : 0.f;
  else
    return 0.f;
}
static inline int get_int(OSCMessage &msg)
{
  if (msg.isFloat(0))
    return msg.getFloat(0);
  else if (msg.isInt(0))
    return msg.getInt(0);
  else if (msg.isBoolean(0))
    return msg.getBoolean(0) ? 1 : 0;
  else
    return 0;
}

static inline bool get_bool(OSCMessage &msg)
{
  if (msg.isFloat(0))
    return msg.getFloat(0) > 0;
  else if (msg.isInt(0))
    return msg.getInt(0) > 0;
  else if (msg.isBoolean(0))
    return msg.getBoolean(0);
  else
    return false;
}

#define read_variable_f(addr, var) \
  inmsg.dispatch(addr, [](OSCMessage &msg) { var = get_float(msg); });

#define read_variable_i(addr, var) \
  inmsg.dispatch(addr, [](OSCMessage &msg) { var = get_int(msg); });

#define read_variable_b(addr, var) \
  inmsg.dispatch(addr, [](OSCMessage &msg) { var = get_bool(msg); });


// FIXME oscquery compressed mode
static void process_osc_input(auto& Udp)
{
  OSCMessage inmsg;
  int size = Udp.parsePacket();

  if (size > 0)
  {
    while (size--)
    {
      inmsg.fill(Udp.read());
    }
    if (!inmsg.hasError())
    {
      read_variable_i("/valve.1/duty", data_model.valve1.duty);
      read_variable_b("/valve.2/duty", data_model.valve2.state);
      read_variable_b("/valve.3/duty", data_model.valve3.state);
      read_variable_i("/valve.4/duty", data_model.valve4.duty);
      read_variable_i("/valve.6/duty", data_model.valve6.duty);
      read_variable_i("/valve.7/duty", data_model.valve7.duty);
      read_variable_i("/valve.8/duty", data_model.valve8.duty);
    }
  }
}
