// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/pubsub/testing/random_names.h"
#include "google/cloud/pubsub/topic_admin_client.h"
#include "google/cloud/pubsub/version.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/random.h"
#include "google/cloud/testing_util/assert_ok.h"
#include "google/cloud/testing_util/is_proto_equal.h"
#include "google/cloud/testing_util/scoped_environment.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace pubsub {
inline namespace GOOGLE_CLOUD_CPP_PUBSUB_NS {
namespace {

using ::google::cloud::testing_util::IsProtoEqual;
using ::google::cloud::testing_util::ScopedEnvironment;
using ::testing::Contains;
using ::testing::Not;

TEST(TopicAdminIntegrationTest, TopicCRUD) {
  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  ASSERT_FALSE(project_id.empty());

  auto topic_names = [](TopicAdminClient client,
                        std::string const& project_id) {
    std::vector<std::string> names;
    for (auto& topic : client.ListTopics(project_id)) {
      EXPECT_STATUS_OK(topic);
      if (!topic) break;
      names.push_back(topic->name());
    }
    return names;
  };

  auto generator = google::cloud::internal::MakeDefaultPRNG();
  Topic topic(project_id, pubsub_testing::RandomTopicId(generator));

  auto publisher =
      TopicAdminClient(MakeTopicAdminConnection(ConnectionOptions{}));

  EXPECT_THAT(topic_names(publisher, project_id),
              Not(Contains(topic.FullName())));

  auto create_response = publisher.CreateTopic(TopicMutationBuilder(topic));
  ASSERT_STATUS_OK(create_response);

  auto get_response = publisher.GetTopic(topic);
  ASSERT_STATUS_OK(get_response);

  EXPECT_THAT(*create_response, IsProtoEqual(*get_response));

  EXPECT_THAT(topic_names(publisher, project_id), Contains(topic.FullName()));

  auto delete_response = publisher.DeleteTopic(topic);
  ASSERT_STATUS_OK(delete_response);

  EXPECT_THAT(topic_names(publisher, project_id),
              Not(Contains(topic.FullName())));
}

TEST(TopicAdminIntegrationTest, CreateTopicFailure) {
  ScopedEnvironment env("PUBSUB_EMULATOR_HOST", "localhost:1");
  auto publisher = TopicAdminClient(MakeTopicAdminConnection());
  auto create_response = publisher.CreateTopic(
      TopicMutationBuilder(Topic("invalid-project", "invalid-topic")));
  ASSERT_FALSE(create_response);
}

TEST(TopicAdminIntegrationTest, ListTopicsFailure) {
  ScopedEnvironment env("PUBSUB_EMULATOR_HOST", "localhost:1");
  auto publisher = TopicAdminClient(MakeTopicAdminConnection());
  auto list = publisher.ListTopics("--invalid-project--");
  auto i = list.begin();
  EXPECT_FALSE(i == list.end());
  EXPECT_FALSE(*i);
}

TEST(TopicAdminIntegrationTest, DeleteTopicFailure) {
  ScopedEnvironment env("PUBSUB_EMULATOR_HOST", "localhost:1");
  auto publisher = TopicAdminClient(MakeTopicAdminConnection());
  auto delete_response =
      publisher.DeleteTopic(Topic("invalid-project", "invalid-topic"));
  ASSERT_FALSE(delete_response.ok());
}

}  // namespace
}  // namespace GOOGLE_CLOUD_CPP_PUBSUB_NS
}  // namespace pubsub
}  // namespace cloud
}  // namespace google
