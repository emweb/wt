#include <iostream>
#include <fstream>
#include <string>

std::string escape(std::string line)
{
  std::string result;

  for (std::string::size_type i = 0; i < line.length(); ++i) {
    if (line[i] == '"')
      result += "\\\"";
    else if (line[i] == '\\')
      result += "\\\\";
    else
      result += line[i];
  }

  return result;
}

int main(int argc, char **argv)
{
  std::ifstream file(argv[1]);
  std::ofstream result(argv[2]);
  std::string varName(argv[3]);

  result << "// This is automatically generated code -- do not edit!" << std::endl
	 << "// Generated from " << argv[1] << std::endl
	 << std::endl
	 << "namespace skeletons {" << std::endl
	 << std::endl
	 << "  const char *" << varName << " =" << std::endl;

  while (file) {
    std::string line;
    std::getline(file, line);
    
    result << "  \"" << escape(line) << "\\r\\n\"" << std::endl;
  }

  result << ";" << std::endl
	 << "}" << std::endl;

  return 0;
}
