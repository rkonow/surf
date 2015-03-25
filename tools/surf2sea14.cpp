#include <sdsl/int_vector.hpp>
#include <iostream>
#include <string>

using namespace sdsl;
using namespace std;

int main(int argc, char* argv[]){
    if ( argc < 3 ){
        cout << "Usage: ./" << argv[0] <<" input_file " <<" output_file" << endl;
        cout << " input_file = [COL]/text_[int_]SURF.sdsl" << endl;
        return 1;
    }
    string input_file = argv[1];
    string output_file = argv[2];
    string prefix = "text_int_";

    if ( input_file.find(prefix) != std::string::npos ) {
        int_vector_buffer<> text(input_file);
        cout<<"text.size()="<<text.size()<<endl;
        int_vector<> output(text.size()-1, 0, text.width());
        for (size_t i=0; i<text.size(); ++i){
            if ( text[i] != 0 ) {
                if ( i+1 < text.size() )
                    output[i] = text[i];
            } else {
                cout << ">>> 0x00 byte at position " << i << endl;
            }           
        }
        store_to_file(output, output_file);
    }
    else {
        int_vector_buffer<8> text(input_file);
        cout<<"text.size()="<<text.size()<<endl;
        ofstream out(output_file);
        for (size_t i=0; i<text.size(); ++i){
            if ( text[i] != 0 ) {
                out << (char)text[i];
            } else {
                cout << "0x00 byte at position " << i << endl;
            }
        }
    }
}
