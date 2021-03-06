// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/pubsub/internal/subscriber_logging.h"
#include "google/cloud/pubsub/testing/mock_subscriber_stub.h"
#include "google/cloud/testing_util/assert_ok.h"
#include "google/cloud/testing_util/capture_log_lines_backend.h"
#include "absl/memory/memory.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace pubsub_internal {
inline namespace GOOGLE_CLOUD_CPP_PUBSUB_NS {
namespace {

using ::testing::AllOf;
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::Return;

class SubscriberLoggingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    backend_ =
        std::make_shared<google::cloud::testing_util::CaptureLogLinesBackend>();
    logger_id_ = google::cloud::LogSink::Instance().AddBackend(backend_);
  }

  void TearDown() override {
    google::cloud::LogSink::Instance().RemoveBackend(logger_id_);
    logger_id_ = 0;
  }

  static Status TransientError() {
    return Status(StatusCode::kUnavailable, "try-again");
  }

  std::shared_ptr<google::cloud::testing_util::CaptureLogLinesBackend> backend_;

 private:
  long logger_id_ = 0;  // NOLINT(google-runtime-int)
};

TEST_F(SubscriberLoggingTest, CreateSubscription) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, CreateSubscription)
      .WillOnce(Return(make_status_or(google::pubsub::v1::Subscription{})));
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  grpc::ClientContext context;
  google::pubsub::v1::Subscription subscription;
  auto status = stub.CreateSubscription(context, subscription);
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines, Contains(HasSubstr("CreateSubscription")));
}

TEST_F(SubscriberLoggingTest, GetSubscription) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, GetSubscription)
      .WillOnce(Return(make_status_or(google::pubsub::v1::Subscription{})));
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  grpc::ClientContext context;
  google::pubsub::v1::GetSubscriptionRequest request;
  auto status = stub.GetSubscription(context, request);
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines, Contains(HasSubstr("GetSubscription")));
}

TEST_F(SubscriberLoggingTest, UpdateSubscription) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, UpdateSubscription)
      .WillOnce(Return(make_status_or(google::pubsub::v1::Subscription{})));
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  grpc::ClientContext context;
  google::pubsub::v1::UpdateSubscriptionRequest request;
  auto status = stub.UpdateSubscription(context, request);
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines, Contains(HasSubstr("UpdateSubscription")));
}

TEST_F(SubscriberLoggingTest, ListSubscriptions) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, ListSubscriptions)
      .WillOnce(Return(
          make_status_or(google::pubsub::v1::ListSubscriptionsResponse{})));
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  grpc::ClientContext context;
  google::pubsub::v1::ListSubscriptionsRequest request;
  request.set_project("test-project-name");
  auto status = stub.ListSubscriptions(context, request);
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines,
              Contains(AllOf(HasSubstr("ListSubscriptions"),
                             HasSubstr("test-project-name"))));
}

TEST_F(SubscriberLoggingTest, DeleteSubscription) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, DeleteSubscription).WillOnce(Return(Status{}));
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  grpc::ClientContext context;
  google::pubsub::v1::DeleteSubscriptionRequest request;
  request.set_subscription("test-subscription-name");
  auto status = stub.DeleteSubscription(context, request);
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines,
              Contains(AllOf(HasSubstr("DeleteSubscription"),
                             HasSubstr("test-subscription-name"))));
}

TEST_F(SubscriberLoggingTest, AsyncPull) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, AsyncPull)
      .WillOnce([](google::cloud::CompletionQueue&,
                   std::unique_ptr<grpc::ClientContext>,
                   google::pubsub::v1::PullRequest const&) {
        return make_ready_future(
            make_status_or(google::pubsub::v1::PullResponse{}));
      });
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  google::cloud::CompletionQueue cq;
  google::pubsub::v1::PullRequest request;
  request.set_subscription("test-subscription-name");
  auto status =
      stub.AsyncPull(cq, absl::make_unique<grpc::ClientContext>(), request)
          .get();
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines,
              Contains(AllOf(HasSubstr("AsyncPull"),
                             HasSubstr("test-subscription-name"))));
}

TEST_F(SubscriberLoggingTest, AsyncAcknowledge) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, AsyncAcknowledge)
      .WillOnce([](google::cloud::CompletionQueue&,
                   std::unique_ptr<grpc::ClientContext>,
                   google::pubsub::v1::AcknowledgeRequest const&) {
        return make_ready_future(Status{});
      });
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  google::cloud::CompletionQueue cq;
  google::pubsub::v1::AcknowledgeRequest request;
  request.set_subscription("test-subscription-name");
  auto status = stub.AsyncAcknowledge(
                        cq, absl::make_unique<grpc::ClientContext>(), request)
                    .get();
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines,
              Contains(AllOf(HasSubstr("AsyncAcknowledge"),
                             HasSubstr("test-subscription-name"))));
}

TEST_F(SubscriberLoggingTest, AsyncModifyAckDeadline) {
  auto mock = std::make_shared<pubsub_testing::MockSubscriberStub>();
  EXPECT_CALL(*mock, AsyncModifyAckDeadline)
      .WillOnce([](google::cloud::CompletionQueue&,
                   std::unique_ptr<grpc::ClientContext>,
                   google::pubsub::v1::ModifyAckDeadlineRequest const&) {
        return make_ready_future(Status{});
      });
  SubscriberLogging stub(mock, TracingOptions{}.SetOptions("single_line_mode"));
  google::cloud::CompletionQueue cq;
  google::pubsub::v1::ModifyAckDeadlineRequest request;
  request.set_subscription("test-subscription-name");
  auto status = stub.AsyncModifyAckDeadline(
                        cq, absl::make_unique<grpc::ClientContext>(), request)
                    .get();
  EXPECT_STATUS_OK(status);
  EXPECT_THAT(backend_->log_lines,
              Contains(AllOf(HasSubstr("AsyncModifyAckDeadline"),
                             HasSubstr("test-subscription-name"))));
}

}  // namespace
}  // namespace GOOGLE_CLOUD_CPP_PUBSUB_NS
}  // namespace pubsub_internal
}  // namespace cloud
}  // namespace google
