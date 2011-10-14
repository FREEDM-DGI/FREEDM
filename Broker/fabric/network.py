import xml.dom.minidom
from StringIO import StringIO

def create_network_file(in_reliability,out_reliability):
    doc = xml.dom.minidom.Document()
    net = doc.createElement("network")
    doc.appendChild(net)
    incoming = doc.createElement("incoming")
    incoming.setAttribute("reliability",str(in_reliability))
    net.appendChild(incoming)
    outgoing = doc.createElement("outgoing")
    net.appendChild(outgoing)
    for uuid,value in out_reliability.iteritems():
        channel = doc.createElement("channel")
        channel.setAttribute("uuid",str(uuid))
        channel.setAttribute("reliabilty",str(value))
        outgoing.appendChild(channel)
    return StringIO(doc.toxml())
