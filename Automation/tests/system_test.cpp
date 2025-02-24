#include <gtest/gtest.h>
#include <cpprest/http_client.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>

using namespace web::http;
using namespace web::http::client;
using namespace web;

// En enkel testfall för att testa ett API-anrop
TEST(SystemTest, APICallTest) {
    http_client client(U("http://jsonplaceholder.typicode.com/posts/1"));
    uri_builder builder(U(""));

    pplx::task<void> requestTask = client.request(methods::GET)
    .then([](http_response response) {
        if (response.status_code() == status_codes::OK) {
            return response.extract_json();
        }
        return pplx::task_from_result(json::value());
    })
    .then([](pplx::task<json::value> previousTask) {
        try {
            const json::value v = previousTask.get(); // Använd en konstant referens
            std::cout << v.serialize() << std::endl; // Använd std::cout
            ASSERT_EQ(v.at(U("id")).as_integer(), 1);
        } catch (http_exception const & e) {
            std::cout << e.what() << std::endl; // Använd std::cout
        }
    });

    try {
        requestTask.wait();
    } catch (std::exception const & e) {
        std::cout << e.what() << std::endl; // Använd std::cout
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
