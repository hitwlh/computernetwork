#include <iostream>
#include <fstream>
using namespace std;

bool copy_binary_file(const char * szDestFile, const char * szOrigFile)  
{  
    if(!szDestFile || !szOrigFile)
        return false;  
    bool bRet = true;
    std::ofstream fout(szDestFile, std::ios::binary);  
    std::ifstream fin(szOrigFile, std::ios::binary);  
    if (fin.bad())
        bRet = false;
    else if(!fin)
        return false;
    else
    {
        char szBuf[256];
        while(!fin.eof())  
        {
            fin.read(szBuf, sizeof(szBuf));
            printf("sizeof(szBuf) is: %d\n", sizeof(szBuf));
            if (fout.bad())
            {
                bRet = false;
                break;  
            }
            //fout.write(szBuf, sizeof(szBuf));
            fout.write(szBuf, fin.gcount());
            //快读到文件末尾的时候，没法写满szBuf，用fin.gcount()方法获取szBuf中有效数据的大小
            //如果正好读到文件最后一个字符时写满szBuf，不会读到eof()符号（就是说如果buf的size是102，文件的大小是3763698个字节，不用gcount()方法，最后写出的文件大小是3763800个字节）

        }
    }  
    fin.close();  
    fout.close();  
    return bRet;  
}
int main()
{
    char *sourcefile, *destfile;
    sourcefile = "/SPclasses/tmp/yul.mp3";
    destfile = "/SPclasses/yul.mp3";
    //sourcefile = "/SPclasses/tmp/a.pdf";
    //destfile = "/SPclasses/a.pdf";
    if(copy_binary_file(destfile, sourcefile))
        cout << "success\n";
    else
        cout << "failed\n";
}