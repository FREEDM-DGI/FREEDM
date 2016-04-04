PROTOBUF_GENERATE_CPP(PROTO_SRCS
                      PROTO_HDRS
                      ProtocolMessage.proto
                      ModuleMessage.proto
                      ClockSynchronizer.proto
                      LoadBalancing.proto
                      GroupManagement.proto
                      StateCollection.proto
                      
                      # for vvc 
                      VoltVarCtrl.proto)

add_library(messages ${PROTO_SRCS} ${PROTO_HDRS})
