/*
Copyright (c) 2018 Johnny Borov <JohnnyBorov@gmail.com>

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <fstream>
#include <string>

void generateResourceDataSourceFile(char* resource_name, char* resource_file_name, char* output_file_name);
void generateResourcesConfigSourceFile(char* resource_names_list, char* config_file_name);
std::string modifyFileName(std::string file_name);


int main(int argc, char* argv[]) {
  // working directory = CMAKE_BINARY_DIR when called from cmake custom command
  if (std::string(argv[1]) == "-data") {
    if (argc == 5)
      generateResourceDataSourceFile(argv[2], argv[3], argv[4]);
    else
      return -1;
  } else if (std::string(argv[1]) == "-config") {
    if (argc == 4)
      generateResourcesConfigSourceFile(argv[2], argv[3]);
    else
      return -1;
  }

  return 0;
}


// generate a cpp file (e.g. resources/res.txt.cpp) containing global extern array of const unsigned char
// of binary data read from the requested input file (CMAKE_SOURCE_DIR/resources/res.txt) plus null-terminator
// in the end of data and a global extern const size_t size of this data in bytes (without null-terminator)
void generateResourceDataSourceFile(char* resource_name, char* resource_file_name, char* output_file_name) {
  std::string name{ resource_name }; // e.g. "resources/res.txt"
  std::string modified_name = modifyFileName(name); // becomes "resources__slash__res__dot__txt"

  std::ifstream ifs{ resource_file_name, std::ios::binary }; // e.g. reads from CMAKE_SOURCE_DIR/resources/res.txt
  std::ofstream ofs{ output_file_name }; // e.g. writes to resources/res.txt.cpp

  ofs << "#include <cstddef>\n";
  ofs << "extern const unsigned char _resource_" << modified_name << "_data[] = {\n";

  int line_count = 0;
  char c;
  while (ifs.get(c)) {
    ofs << "0x" << std::hex << (c & 0xff) << ", ";

    if (++line_count == 10) {
      ofs << '\n';
      line_count = 0;
    }
  }

  ofs << "\'\\0\'"; // null-terminator in case data is going to be interprited as a c string
  ofs << "};\n";
  // -1 excludes null-terminator
  ofs << "extern const size_t _resource_" << modified_name << "_len = sizeof(_resource_" << modified_name << "_data) - 1;\n";
}


// generate a cpp file (e.g. __resources__config.cpp) that defines ResourceHandle constructor.
// The defenition contains the hardcoded mapping of original resource names (e.g. "resources/res.txt")
// to their corresponding global extern variables names. Thus it returns a handle which contains pointer to
// the beginning of global extern array of const unsigned char and its size (see -data option description)
void generateResourcesConfigSourceFile(char* resource_names_list, char* config_file_name) {
  std::ofstream ofs(config_file_name); // e.g. writes to __resources__config.cpp

  ofs <<
    "#include \"ResourceHandle.h\"\n"
    "\n"
    "ResourceHandle::ResourceHandle(std::string resource_name) {\n"
    "  ";


  std::string resource_names{resource_names_list}; // e.g. "res.txt;res2.txt;resources/res.txt"
  size_t length = resource_names.length();
  size_t start_pos = 0;
  do {
    size_t end_pos = resource_names.find_first_of(';', start_pos);
    if (end_pos == std::string::npos)
      end_pos = length;

    // e.g. "res.txt;res2.txt;resources/res.txt" -> "res.txt" -> "res__dot__txt"
    std::string name = resource_names.substr(start_pos, end_pos - start_pos);
    std::string modified_name = modifyFileName(name);

    ofs <<
      "if (resource_name == \"" << name << "\") {\n"
      "    extern const unsigned char _resource_" << modified_name << "_data[];\n"
      "    extern const size_t _resource_" << modified_name << "_len;\n"
      "    m_data_start = _resource_" << modified_name << "_data;\n"
      "    m_data_len = _resource_" << modified_name << "_len;\n"
      "  } else ";

    start_pos = end_pos + 1; // e.g. start next read from res2... position in "res.txt;res2.txt;resources/res.txt"
  } while (start_pos < length);


  ofs <<
    "{\n"
    "#ifdef RM_NO_EXCEPTIONS\n"
    "    m_data_start = nullptr;\n"
    "    m_data_len = 0;\n"
    "#else\n"
    "    throw ResourceNotFound{resource_name};\n"
    "#endif\n"
    "  }\n"
    "}\n";
}


// replace symbols that cant be used in c++ identeficator name
// e.g. "resources/res.txt" -> "resources__slash__res__dot__txt"
std::string modifyFileName(std::string file_name) {
  size_t search_from_pos = 0;
  size_t replace_from_pos;
   while ((replace_from_pos = file_name.find_first_of(".- /\\", search_from_pos)) != std::string::npos) {
     switch (file_name[replace_from_pos]) {
     case '.':
       file_name.replace(replace_from_pos, 1, "__dot__");
       search_from_pos = replace_from_pos + 7; // shift len(__dot__) = 5 symbols
       break;
     case '-':
       file_name.replace(replace_from_pos, 1, "__dash__");
       search_from_pos = replace_from_pos + 8;
       break;
     case ' ':
       file_name.replace(replace_from_pos, 1, "__space__");
       search_from_pos = replace_from_pos + 9;
       break;
     case '/':
       file_name.replace(replace_from_pos, 1, "__slash__");
       search_from_pos = replace_from_pos + 9;
       break;
     case '\\':
       file_name.replace(replace_from_pos, 1, "__bslash__");
       search_from_pos = replace_from_pos + 10;
       break;
     }
  }

  return file_name;
}