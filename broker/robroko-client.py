from nanomsg import Socket, PUB, SUB, SUB_SUBSCRIBE

host = "tcp://localhost:14040"

with Socket(SUB) as sock:
    print sock.connect(host)
    print sock

    # subscribe to errything
    sock.set_string_option(SUB, SUB_SUBSCRIBE, '')

    while True:
        packet = sock.recv()
        print packet
