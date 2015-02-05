import select, socket
from nanomsg import Socket, PUB, SUB, SUB_SUBSCRIBE
import pybonjour

queried = []
resolved = []
timeout = 5


def nanomsg_connect(host, port):
    # pybonjour delivers hostnames with an additional period. We're removing that to get something
    # that ends on .local (and not .local.)
    nanomsg_host = "tcp://%s:%s" % (host[:-1], port)
    print "Connecting to %s" % nanomsg_host
    
    with Socket(SUB) as sock:
        sock.connect(nanomsg_host)
        # subscribe to errything
        sock.set_string_option(SUB, SUB_SUBSCRIBE, '')

        while True:
            packet = sock.recv()
            print packet

# def resolve_dns(sdRef, flags, interfaeeIndex, errorCode, fullname, rrtype, rrclass, rdata, ttl):
#     print "IP %s" % socket.inet_ntoa(rdata)

def resolve_callback(sdRef, flags, interfaceIndex, errorCode, fullname, hosttarget, port, txtRecord):
    if errorCode != pybonjour.kDNSServiceErr_NoError:
        return

    print "Located %s on target %s on port %s " % (fullname, hosttarget, port)

    nanomsg_connect(hosttarget, port)
    
    # query_sdRef = pybonjour.DNSServiceQueryRecord(
    #     interfaceIndex = interfaceIndex,
    #     fullname = hosttarget,
    #     rrtype = pybonjour.kDNSServiceType_A,
    #     callBack = resolve_dns)  
    
    # try:
    #     while not queried:
    #         ready = select.select([query_sdRef], [], [], timeout)
    #         if query_sdRef not in ready[0]:
    #             logger.warn('Query record timed out')
    #             break
    #         pybonjour.DNSServiceProcessResult(query_sdRef)
    #     else:
    #         queried.pop()
    # finally:
    #     query_sdRef.close()

    resolved.append(True)

def avahi_resolve(sdRef, flags, interfaceIndex, errorCode, serviceName, regtype, replyDomain):
    if errorCode != pybonjour.kDNSServiceErr_NoError:
        return
    if not (flags & pybonjour.kDNSServiceFlagsAdd):
        return

    # No problems, let's resolve
    avahi_resolver = pybonjour.DNSServiceResolve(0,
                                                 interfaceIndex,
                                                 serviceName,
                                                 regtype,
                                                 replyDomain,
                                                 resolve_callback)

    try:
        while not resolved:
            ready = select.select([avahi_resolver], [], [], timeout)
            if avahi_resolver not in ready[0]:
                break
            pybonjour.DNSServiceProcessResult(avahi_resolver)
        else:
            resolved.pop()
    finally:
          avahi_resolver.close()



avahi_browser = pybonjour.DNSServiceBrowse(regtype = "_rokoko._tcp", callBack = avahi_resolve)

try:
    try:
        while True:
            ready = select.select([avahi_browser], [], [])
            if avahi_browser in ready[0]:
                pybonjour.DNSServiceProcessResult(avahi_browser)
    except KeyboardInterrupt:
        pass
finally:
    avahi_browser.close()



# STEP 2: CONNECT TO THE SERVICE
