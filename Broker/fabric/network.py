import xml.dom.minidom
from StringIO import StringIO

def create_network_file(in_reliability,out_reliability):
    doc = xml.dom.minidom.Document()
    net = doc.createElement("network")
    doc.appendChild(net)
    incoming = doc.createElement("incoming")
    reliability = doc.createElement("reliability")
    reliability.appendChild(doc.createTextNode(str(in_reliability)))
    incoming.appendChild(reliability)
    net.appendChild(incoming)
    outgoing = doc.createElement("outgoing")
    net.appendChild(outgoing)
    for uuid,value in out_reliability.iteritems():
        channel = doc.createElement("channel")
        uuidattr = doc.createElement("uuid")
        uuidattr.appendChild(doc.createTextNode(str(uuid)))
        reliability = doc.createElement("reliability")
        reliability.appendChild(doc.createTextNode(str(value)))
        #channel.appendChild(uuidattr)
        channel.setAttribute('uuid',uuid)
        channel.appendChild(reliability)
        outgoing.appendChild(channel)
    return StringIO(doc.toxml())
