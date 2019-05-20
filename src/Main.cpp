#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>

struct Node
{
    char ch;
    unsigned int count;

    Node* left_child;
    Node* right_child;

    Node(char ch, unsigned int count)
        : ch(ch)
        , count(count)
        , left_child(NULL)
        , right_child(NULL)
    {

    }
};

unsigned int get_file_size(const std::string& filepath)
{
    std::ifstream in_file(filepath);

    unsigned int byte_count = 0;

    char ch;
    while(in_file.get(ch))
        byte_count++;

    return byte_count;
}

std::string change_file_suffix(const std::string& filepath, const std::string& suffix)
{
    std::string new_name("");

    for(int i = 0; filepath[i] != '.'; i++)
        new_name += filepath[i];

    return new_name + '.' + suffix;
}

Node* make_huffman_tree(std::vector<Node*>& free_nodes)
{

    while(free_nodes.size() > 1)
    {
        std::stable_sort(free_nodes.begin(), free_nodes.end(), [](Node* x, Node* y) -> bool
        {
            return x->count < y->count;
        });

        Node* node_1 = free_nodes[0];
        Node* node_2 = free_nodes[1];

        Node* parent = new Node('*', node_1->count + node_2->count);

        parent->left_child = node_1;
        parent->right_child = node_2;

        free_nodes.erase(free_nodes.begin());
        free_nodes.erase(free_nodes.begin());

        free_nodes.push_back(parent);

    }

    return free_nodes[0];
}

void code_nodes(Node* node, std::string binary, std::map<std::string, char>& char_code_lookup)
{
    if(node->left_child)
    {
        std::string temp = binary + "0";
        code_nodes(node->left_child, temp, char_code_lookup);
    }
    if(node->right_child)
    {
        std::string temp = binary + "1";
        code_nodes(node->right_child, temp, char_code_lookup);
    }


    if(!node->left_child && !node->right_child)
    {
        char_code_lookup.insert({binary, node->ch});
    }
}

void code_nodes(Node* node, std::string binary, std::map<char, std::string>& char_code_lookup)
{
    if(node->left_child)
    {
        std::string temp = binary + "0";
        code_nodes(node->left_child, temp, char_code_lookup);
    }
    if(node->right_child)
    {
        std::string temp = binary + "1";
        code_nodes(node->right_child, temp, char_code_lookup);
    }


    if(!node->left_child && !node->right_child)
    {
        char_code_lookup.insert({node->ch, binary});
    }
}

int get_compression_ratio(const std::string& filepath)
{
    float size = get_file_size(filepath);
    float new_size = get_file_size(change_file_suffix(filepath, "cmp"));

    float ratio = (new_size/size) * 100.0f;

    return 100 - (int)ratio;
}

void compress(const std::string& filepath)
{
    std::string suffix("");
    for(int i = filepath.size()-1; filepath[i] != '.'; i--)
    {
        suffix += filepath[i];
    }
    unsigned int p = suffix.size()-1;
    for(int i = 0; i < p; i++)
    {
        std::swap(suffix[i], suffix[p]);
        p--;
    }

    unsigned int file_size = get_file_size(filepath);
    char* in_buffer = new char[file_size];

    std::ifstream in_file(filepath, std::ios::binary);

    in_file.read(in_buffer, file_size);

    in_file.close();

    std::vector<Node*> free_nodes;

    for(int i = 0; i < file_size; i++)
    {
        char ch = in_buffer[i];
        bool is_new = true;
        for(Node* node : free_nodes)
        {
            if(node->ch == ch)
            {
                node->count++;
                is_new = false;
                break;
            }
        }
        if(is_new)
            free_nodes.push_back(new Node(ch, 1));

    }

    std::string new_filepath(change_file_suffix(filepath, "cmp"));

    std::ofstream out_file(new_filepath, std::ios::binary);

    out_file << suffix << " ";

    out_file << free_nodes.size() << " ";

    std::sort(free_nodes.begin(), free_nodes.end(), [](Node* x, Node* y)
    {
        return x->count > y->count;
    });

    for(Node* node : free_nodes)
    {
        out_file << node->ch << " " << node->count << " ";
    }


    Node* root = make_huffman_tree(free_nodes);

    std::map<char, std::string> char_code_lookup;

    code_nodes(root, "", char_code_lookup);

    std::string binary;
    binary.reserve(file_size * 8);

    for(int i = 0; i < file_size; i++)
    {
        char ch = in_buffer[i];

        binary += char_code_lookup[ch];
    }

    char* out_buffer = new char[file_size];

    unsigned int byte_count = 0;
    for(int i = 0; i < binary.size();)
    {
        for(int p = 0; p < 8; p++)
        {
            bool bit = binary[i] == '1' ? 1:0;
            out_buffer[byte_count] ^= (-bit ^ out_buffer[byte_count]) & (1UL << p);
            i++;
        }
        byte_count++;
    }

    out_file << byte_count << " ";

    out_file.write(out_buffer, byte_count);


}

void decompress(const std::string& filepath)
{
    std::ifstream in_file(filepath, std::ios::binary);

    std::string suffix;
    in_file >> suffix;

    unsigned int unique_char_count;
    in_file >> unique_char_count;



    std::vector<Node*> free_nodes;
    for(int i = 0; i < unique_char_count; i++)
    {
        char ch;
        in_file.get(ch);
        in_file.get(ch);

        unsigned int num;
        in_file >> num;

        free_nodes.push_back(new Node(ch, num));
    }


    Node* root = make_huffman_tree(free_nodes);

    std::map<std::string, char> char_code_lookup;


    code_nodes(root, "", char_code_lookup);

    unsigned int byte_count;
    in_file >> byte_count;
    char ch;
    in_file.get(ch);

    char* in_buffer = new char[byte_count];

    in_file.read(in_buffer, byte_count);


    std::string new_filepath(change_file_suffix(filepath, suffix));

    char* out_buffer = new char[byte_count * 2];

    std::cout << "MAKING CHARS\n";

    unsigned int new_byte_count = 0;
    std::string binary("");
    for(int i = 0; i < byte_count; i++)
    {
        for(int p = 0; p < 8; p++)
        {
            bool bit = (in_buffer[i] >> p) & 1;

            binary += bit ? '1':'0';

            //bool is_char = false;
            //for(auto itr : char_code_lookup)
            //{
            //    if(itr.first == binary)
            //        is_char = true;
            //}

            if(char_code_lookup.find(binary) != char_code_lookup.end())
            {
                out_buffer[new_byte_count] = char_code_lookup[binary];
                new_byte_count++;
                binary = "";
                if(i == byte_count-1)
                    break;
            }

        }

    }

    std::ofstream out_file(new_filepath, std::ios::binary);

    out_file.write(out_buffer, new_byte_count);

}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Program takes 1 argument!\n");
        printf("file_compressor [filepath]\n");
        return 0;
    }

    std::string filepath(argv[1]);

    if(filepath.find(".cmp") == std::string::npos)
    {
        compress(filepath);
        int ratio = get_compression_ratio(filepath);
        if(ratio < 0)
        {
            std::cout << "Compression not possible on this file.\nNumber of same bytes to small.\n";
            remove(change_file_suffix(filepath, "cmp").c_str());
            return 0;
        }
        std::cout << "Compressed by " << ratio << "%\n";
    }
    else
    {
        decompress(filepath);
        std::cout << "Decompressed\n";
    }

    return 0;
}
