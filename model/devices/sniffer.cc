/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sniffer.h"

#include "model/setup/device_boutique.h"
#include "os/log.h"

using std::vector;

namespace rootcanal {

bool Sniffer::registered_ =
    DeviceBoutique::Register("sniffer", &Sniffer::Create);

Sniffer::Sniffer() {}

void Sniffer::Initialize(const vector<std::string>& args) {
  if (args.size() < 2) return;

  if (Address::FromString(args[1], device_to_sniff_)) {
    properties_.SetAddress(device_to_sniff_);
  }

  if (args.size() < 3) return;
}

void Sniffer::TimerTick() {}

void Sniffer::IncomingPacket(model::packets::LinkLayerPacketView packet) {
  Address source = packet.GetSourceAddress();
  Address dest = packet.GetDestinationAddress();
  bool match_source = device_to_sniff_ == source;
  bool match_dest = device_to_sniff_ == dest;
  if (!match_source && !match_dest) {
    return;
  }
  model::packets::PacketType packet_type = packet.GetType();

  if (packet_type == model::packets::PacketType::RSSI_WRAPPER) {
    auto wrapper_view = model::packets::RssiWrapperView::Create(packet);
    ASSERT(wrapper_view.IsValid());
    auto wrapped_view =
        model::packets::LinkLayerPacketView::Create(wrapper_view.GetPayload());
    ASSERT(wrapped_view.IsValid());
    LOG_INFO(
        "%s %s -> %s (Type %s wrapping %s)",
        (match_source ? (match_dest ? "<->" : "<--") : "-->"),
        source.ToString().c_str(), dest.ToString().c_str(),
        model::packets::PacketTypeText(packet_type).c_str(),
        (packet_type == model::packets::PacketType::RSSI_WRAPPER
             ? model::packets::PacketTypeText(wrapped_view.GetType()).c_str()
             : ""));
  } else {
    LOG_INFO("%s %s -> %s (Type %s)",
             (match_source ? (match_dest ? "<->" : "<--") : "-->"),
             source.ToString().c_str(), dest.ToString().c_str(),
             model::packets::PacketTypeText(packet_type).c_str());
  }
}

}  // namespace rootcanal
