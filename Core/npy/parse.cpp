#include "parse.h"



int main(int argc, char**argv) {
	npy_header nh;
	if (argc > 1) {
		parse_npy(argv[1], &nh);
		std::cout << "PROVA " << nh.header_length << std::endl;
		std::cout << "PROVA " << nh.bit << std::endl;
		return 0;
	}
	else return -1;
}

int parse_npy(char*fname, npy_header * head){
	std::ifstream ifs;

	ifs.open(fname, std::ifstream::in);
	char magic[6];
	char major, minor;
	int offset = 0, header_length = 0;
	char header_v1[2];
	char header_v2[4];
	ifs.read(magic, 6);
	offset += 6;
	//std::cout << "Magic string: <" << magic << ">" << std::endl;
	ifs.read(&major, 1);
	offset++;
	ifs.read(&minor, 1);
	offset++;

	//std::cout << "Version: " << (short)(major) << "." << (short)(minor) << std::endl;

	if (major == 1) {
		ifs.read(header_v1, 2);
		//std::cout << "header: " << *(reinterpret_cast<unsigned short*>(header_v1)) << std::endl;
		offset += 2;
		header_length = *(reinterpret_cast<unsigned short*>(header_v1));
	}
	else if (major == 2)
	{
		ifs.read(header_v2, 4);
		//std::cout << "header: " << *(reinterpret_cast<unsigned int*>(header_v2)) << std::endl;
		offset += 4;
		header_length = *(reinterpret_cast<unsigned int*>(header_v2));
	}
	else {
		return -1;
	}

	offset += header_length;
	int j = 0;
	char *the_header;
	the_header = (char *)malloc(header_length * sizeof(char));
	std::vector<std::string> keys;
	std::string key = std::string();
	std::vector<std::string> values;
	std::string val = std::string();
	std::vector<std::string> tuple;
	int parsing = 0;
	int parse_key = 0; int parse_value = 0;
	int istuple = 0; int istruefalse = 0; int isstring = 0;
	// { 0x7b
	// } 0x7d
	// space 0x20
	// ' 0x27
	// : 0x3a
	// , 0x2c
	// ( 0x28
	// ) 0x29

	while (j < header_length) {
		char c = ifs.get();
		the_header[j] = c;
		//std::cout << "read character " << c << std::endl;

		if (c == '{' && parse_key == 0 && parse_value == 0) {
			// it means we are at the beginning
			parse_key = 1;
			parse_value = 0;
		}

		if (parse_key == 1) {
			//std::cout << "parse_key " << c << std::endl;
			if (c == ' ' || c == '{' || c == ',') {
				//std::cout << "skip" << std::endl;
			}
			else if (c == 0x27) {
				if (parsing == 0) {
					//std::cout << "parse_key parsing  0 -> 1" << std::endl;
					parsing = 1;
				}
				else {
					//std::cout << "parse_key parsing  1 -> 0" << std::endl;
					parsing = 0;
					keys.push_back(key);
					std::cout << "key = " << key <<  std::endl;
					key.clear();
				}
			}
			else if (c == ':') {
				//std::cout << "parse_key -> 0 parse_value -> 1" << std::endl;
				parse_key = 0;
				parse_value = 1;
			}
			else {
				//std::cout << "append char" << std::endl;
				key.append(1, c);
			}
		}
		// { 0x7b
		// } 0x7d
		// space 0x20
		// ' 0x27
		// : 0x3a
		// , 0x2c
		// ( 0x28
		// ) 0x29
		if (parse_value == 1) {
			if (c == ' ' ) {
				//skip
			}
			else {
				if (c == ':') {
					//parsing = 0;
				}
				else if ((c == 0x27 )) {
					if (parsing == 0) {
						parsing = 1;
						if (isstring == 0) {
							isstring = 1;
							//val.append(c, 1);
						}
						else if (isstring == 1) {
							isstring = 0;
							parsing = 0;
							values.push_back(val);
							std::cout << "val = " << val << std::endl;
							val.clear();
							parse_key = 1;
							parse_value = 0;
						} else if (istuple == 1) {
							//std::cout << "parsed up to : " << val ;
							//val.append( 1,c);
							tuple.push_back(val);
							val.clear();
							parsing = 0;
							//std::cout << "added " << c << " " << val << std::endl;
						}
					}
					else {
						if (istuple == 1) {
							//std::cout << "parsed up to : " << val ;
							//val.append( 1,c);
							tuple.push_back(val);
							val.clear();
							//std::cout << "added " << c << " " << val << std::endl;
						}
						else {
							parsing = 0;
							values.push_back(val);
							std::cout << "val = " << val << std::endl;
							val.clear();
							parse_key = 1;
							parse_value = 0;
						}
					}

				}
				else if (c == ',') {
					if (istruefalse == 1) {
						istruefalse = 0;
						values.push_back(val);
						std::cout << "val = " << val << std::endl;
						val.clear();
						parse_key = 1;
						parse_value = 0;
					}
					else if (istuple == 1) {
						//val.append(c, 1);
						tuple.push_back(val);
						val.clear();
					}
				}
				else if (c == '(') {
					istuple = 1;
					parsing = 1;
					//val.append(1, c);
				}
				else if (c == 'f' || c == 'F' || c == 't' || c == 'T') {
					istruefalse = 1;
					val.append(1, c);
				}
				else if (c == ')') {
					//val.append(1, c);
					istuple = 0;
					tuple.push_back(val);
					parsing = 0;
					std::string str{ "tuple" };
					values.push_back(str);
					std::cout << "val = " << val << std::endl;
					val.clear();
					parse_key = 1;
					parse_value = 0;
				}
				else {
					val.append(1, c);
				}
			}
		}

		j++;
	}
	std::cout << std::endl;

	
	for (auto val : values) {
		std::cout << "val " << val << std::endl;
	}
	
	for (auto key : keys) {
		std::cout << "key " << key << std::endl;
	}
	for (auto key : tuple) {
		std::cout << "tuple :" << key << std::endl;
	}
	
	ifs.close();
	// pass the header to Python as it is easier to handle text
	// TODO: handle it here
	//std::string out(the_header); 
	//std::cout << out << std::endl;
	free(the_header);

	//npy_header head;
	head->X = std::stoi(tuple[0]);
	head->Y = std::stoi(tuple[1]);
	head->Z = std::stoi(tuple[2]);
	head->isBigEndian = values[0][0] == '<' ? false : true;
	head->header_length = header_length;
	head->isFortranOrder = values[1][0] == 'T' ? true : false;
	head->major = major;
	head->minor = minor;
	if (values[0][2] == '1') {
		head->bit = 8;
		std::cout << "8 bit" << std::endl;
	}
	else if (values[0][2] == '2'){
		head->bit = 16;
		std::cout << "16 bit" << std::endl;
	}
	else {
		std::cout << "error parsing bit" << std::endl;
	}
	head->the_header =  the_header;


	return 0;
}
