//
// Created by jzielins on 22.01.2022.
//
#include "rapidjson/document.h"
#include <string>
#include <iostream>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace  std;
using namespace rapidjson;
int main(){
    // 1. Parse a JSON text string to a document.

    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
    printf("Original JSON:\n %s\n", json);

    Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.

    printf("\nParsing to document succeeded.\n");
    StringBuffer buffer1;
    Writer<StringBuffer> writer(buffer1);

    document.Accept(writer);
    std::cout << buffer1.GetString();
    return 0;
}