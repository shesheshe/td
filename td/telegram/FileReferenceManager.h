//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2019
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/actor/actor.h"
#include "td/actor/PromiseFuture.h"

#include "td/telegram/ChatId.h"
#include "td/telegram/ChannelId.h"
#include "td/telegram/files/FileManager.h"
#include "td/telegram/MessageId.h"
#include "td/telegram/SetWithPosition.h"
#include "td/telegram/UserId.h"

#include "td/utils/Variant.h"

#include <unordered_map>

namespace td {

extern int VERBOSITY_NAME(file_references);

class FileReferenceManager : public Actor {
  struct Node;

 public:
  FileSourceId create_message_file_source(FullMessageId full_message_id);

  using NodeId = FileId;
  void update_file_reference(NodeId node_id, Promise<> promise);
  void add_file_source(NodeId node_id, FileSourceId file_source_id);
  void remove_file_source(NodeId file_id, FileSourceId file_source_id);
  void merge(NodeId to_node_id, NodeId from_node_id);

 private:
  struct Destination {
    bool empty() const {
      return node_id.empty();
    }
    NodeId node_id;
    int64 generation;
  };
  struct Query {
    std::vector<Promise<>> promises;
    int32 active_queries{0};
    Destination proxy;
    int64 generation;
  };

  struct Node {
    SetWithPosition<FileSourceId> file_source_ids;
    unique_ptr<Query> query;
  };

  struct FileSourceMessage {
    FullMessageId full_message_id;
  };
  struct FileSourceUserPhoto {
    int64 photo_id;
    UserId user_id;
  };
  struct FileSourceChatPhoto {
    ChatId chat_id;
  };
  struct FileSourceChannelPhoto {
    ChannelId channel_id;
  };
  struct FileSourceWallpapers {
    // empty
  };
  struct FileSourceWebPage {
    string url;
  };
  struct FileSourceSavedAnimations {
    // empty
  };

  using FileSource = Variant<FileSourceMessage, FileSourceUserPhoto, FileSourceChatPhoto, FileSourceChannelPhoto,
                             FileSourceWallpapers, FileSourceWebPage, FileSourceSavedAnimations>;
  vector<FileSource> file_sources_;

  int32 last_file_source_id_{0};
  int64 query_generation{0};

  std::unordered_map<NodeId, Node, FileIdHash> nodes_;

  void run_node(NodeId node);
  void send_query(Destination dest, FileSourceId file_source_id);
  Destination on_query_result(Destination dest, FileSourceId file_source_id, Status status, int32 sub = 0);
};

}  // namespace td
