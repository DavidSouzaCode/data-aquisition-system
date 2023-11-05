#include <iostream>
#include <boost/asio.hpp>
#include <cstdlib>
#include <memory>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <vector>

using boost::asio::ip::tcp;

std::vector<std::vector<std::string>> data;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    read_message();
  }

private:
  void read_message()
  {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, buffer_, "\r\n",
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          std::vector<int> pos;
          std::string to_client;
          if (!ec)
          {
            std::istream is(&buffer_);
            std::string message(std::istreambuf_iterator<char>(is), {});

            if (boost::algorithm::starts_with(message, "LOG")) {
                
                std::vector<std::string> parts;
                boost::split(parts, message, boost::is_any_of("|"));
                parts.erase(parts.begin());
                data.push_back(parts);
                to_client = "Message received!";

            }
            else if(boost::algorithm::starts_with(message, "GET")){

                std::vector<std::string> parts;
                boost::split(parts, message, boost::is_any_of("|"));
                parts.erase(parts.begin());
                for(int i = 0; i < data.size(); i++){
                    if(parts[0] == data[i][0]){
                        pos.push_back(i);
                    }          
                }
                if(pos.empty()){
                  to_client = "ERROR|INVALID_SENSOR_ID\r\n.";
                }
                else{
                  int num_records = std::stoi(parts[1]);
                  int num_available = pos.size();
                  int num_to_send = std::min(num_records, num_available);
                  to_client = std::to_string(num_to_send) + ";";
                  for(int i = num_available - num_to_send; i < num_available; i++){
                      to_client += data[pos[i]][1] + "|" + data[pos[i]][2];
                      if(i+1 < num_available){
                          to_client += ";";
                      }
                  }
                  to_client += "\r\n.";
                }
            }

            std::cout << "Received: " << message << std::endl;
            write_message(to_client);
          }
        });
  }

  void write_message(const std::string& message)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(message),
        [this, self, message](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            read_message();
          }
        });
  }

  tcp::socket socket_;
  boost::asio::streambuf buffer_;
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    accept();
  }

private:
  void accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket))->start();
          }

          accept();
        });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{

  boost::asio::io_context io_context;

  server s(io_context, 9000);

  io_context.run();

  return 0;
}


/*
main 
developer@codespaces-2c5b73:/workspaces$ cd data-aquisition-system/
developer@codespaces-2c5b73:/workspaces/data-aquisition-system$ cd build
developer@codespaces-2c5b73:/workspaces/data-aquisition-system/build$ 

developer@codespaces-2c5b73:/workspaces$ cd data-aquisition-system/
developer@codespaces-2c5b73:/workspaces/data-aquisition-system$ cd emulators
developer@codespaces-2c5b73:/workspaces/data-aquisition-system/emulators$ 
*/