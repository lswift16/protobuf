#include <string>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <fstream>
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/dynamic_message.h>

#include "protoHandler.hpp"

using namespace google::protobuf;

void ProtoHandler::protoMethod(std::string &request){
	//  cfile is a c file descriptor (not to be confused with a protobuf FileDescriptor object)
    int cfile = open("allProto.desc", O_RDONLY);
	
    FileDescriptorSet fds;

//  Parse a FileDescriptorSet object directly from the file
//  Has the format of a protobuf Message - subclass FileDescriptorSet, defined in <google/protobuf/descriptor.pb.h>
//  https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.descriptor.pb#FieldOptions_CType.details
    fds.ParseFromFileDescriptor(cfile);
		
//  Use FileDescriptorSet method to print to screen
//  fds.SerializeToOstream(&cout);
	
	close(cfile);
	
// A DescriptorPool is required: provides methods to identify and manage message types at run-time
// DescriptorPool can be populated from a SimpleDescriptorDatabase, which can be populated with FileDescriptorProto objects
    SimpleDescriptorDatabase sddb;
    for ( int i = 0; i < fds.file_size() ; i++ ){
	   //Iterate over the "file" collection in the FileDescriptorSet
	   //Populate the sddb
       sddb.Add(fds.file(i));
    }
	
// Now construct the DescriptorPool
    DescriptorPool dp(&sddb);

// DynamicMessageFactory is constucted from a populated DescriptorPool
// DescriptorPool, Descriptor, FieldDescriptor etc.: see descriptor.h  - 
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.descriptor
    DynamicMessageFactory dmf(&dp);
	
    const Descriptor* desc;
	const Descriptor* payload_desc;
	
    desc = dp.FindMessageTypeByName("DescribedMessage");

// Example of dynamically creating a message from a Descriptor, retrieved by name string
    Message *msg = dmf.GetPrototype(desc)->New();
	msg->ParseFromArray(request.data(),request.size());
		
// Messages with required fields - Need populated. 
// Requires FieldDescriptor objects to access
    const FieldDescriptor* nameField = desc->FindFieldByName("full_name");
	const FieldDescriptor* dataField = desc->FindFieldByName("message");

	
// Reflection object provides R/W access to dynamic message fields, using FieldDescriptors
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.message#Message.GetReflection.details
// Good example of Reflection at top of that page

    const Reflection *msgRefl = msg->GetReflection();

//  Make payload message
	payload_desc = dp.FindMessageTypeByName(msgRefl->GetString(*msg, nameField));
	Message *payload_msg = dmf.GetPrototype(payload_desc)->New();
	payload_msg->ParseFromString(msgRefl->GetString( *msg, dataField));
	
// Reflection of payload message
	const Reflection *main_msgRefl = payload_msg->GetReflection();
	
// Payload fielddescriptors
	const FieldDescriptor* main_debugField = payload_desc->FindFieldByName("debug");
	main_msgRefl->SetString (payload_msg,main_debugField,"Read");
	std::cout << "Payload Read" << endl;
	
// Put the payload data back into the envelope 
	string payload_data;
	payload_msg->SerializeToString(&payload_data);
	msgRefl->SetString(msg, dataField, payload_data);
	
// Now that required fields are populated, the dynamic message can be serialized and printed out.
    string data;
    msg->SerializeToString(&data);
	request=data;


// put data back into message to be replied 
	//memcpy(request.data(), data.c_str(), data.length());

// Useful examples of dynamic protobuf logic here : http://www.programmershare.com/2803644/
// (English not very good)
// 3.4 also describes dynamic compilation Uses google::protobuf::compiler Importer class
// Another link : dynamic stuff : 
//  http://stackoverflow.com/questions/11996557/how-to-dynamically-build-a-new-protobuf-from-a-set-of-already-defined-descriptor
//  https://bitbucket.org/g19fanatic/prototest/src/dfd73a577dcc9bb51cbb3e99319cad352bfc60a8/src/main.cpp?at=master&fileviewer=file-view-default
	
	
}