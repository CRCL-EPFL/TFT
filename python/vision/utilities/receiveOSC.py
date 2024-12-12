from pythonosc import dispatcher
from pythonosc import osc_server
import argparse

def print_handler(address, *args):
    print(f"Received message at {address}: {args}")

if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip",
        default="127.0.0.1", help="The ip to listen on")
    parser.add_argument("--port",
        type=int, default=7777, help="The port to listen on")
    args = parser.parse_args()

    # Set up dispatcher and server
    dispatcher = dispatcher.Dispatcher()
    dispatcher.set_default_handler(print_handler)

    server = osc_server.ThreadingOSCUDPServer(
        (args.ip, args.port), dispatcher)
    
    print(f"Serving on {server.server_address}")
    server.serve_forever()