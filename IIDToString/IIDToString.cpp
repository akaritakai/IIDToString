// IIDToString.cpp :
//   Testing two ways to convert an IID to a string, as described in Dave Cutler's YouTube video here:
//   https://www.youtube.com/watch?v=VYTF4KIF2z0
//

#include <iostream>
#include <combaseapi.h>

auto constexpr TEST_ITERATIONS = 1000000;

// Basic function to convert an IID to a string representation using printf.
void IIDToString_Printf(const IID* iid, char* out)
{
  #pragma warning(suppress : 4996)
  sprintf(out, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            iid->Data1, iid->Data2, iid->Data3,
            iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
            iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
}

// Faster function to convert an IID to a string representation using lookup tables etc.
void IIDToString_Fast(const IID* iid, char* out)
{
    static const unsigned char order[16] = {
        3,2,1,0, 5,4, 7,6, 8,9,10,11,12,13,14,15
    };

    static const unsigned char hex[16] = {
        '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
    };

    const unsigned char *p = (const unsigned char *)iid;
    int i, j = 0;

    for (i = 0; i < 16; i++)
    {
        unsigned char b = p[order[i]];

        out[j++] = hex[b >> 4];
        out[j++] = hex[b & 0xF];

        if (j == 8 || j == 13 || j == 18 || j == 23)
            out[j++] = '-';
    }

    out[j] = '\0';
}

// Function taking a pointer to a suitable IIDToString function and testing it with a large number of iterations to measure performance.
ULONGLONG testIt(void (*func)(const IID* iid, char* out), const IID* iid, int iterations, char* buffer) {
    auto start = GetTickCount64();
    
    for (int i = 0; i < iterations; i++) {
        func(iid, buffer);
    }
    
    return GetTickCount64() - start;
}

int main()
{
  IID testIID;
  HRESULT res;
  
  res = IIDFromString(L"{cecec95a-d894-491a-bee3-5e106fb59f2d}", &testIID);

  if (FAILED(res)) {
    std::cerr << "Failed to create IID from string. Error code: " << res << "\n";
    return 1;
  }

  char buffer[64];

  auto dur = testIt(IIDToString_Printf, &testIID, TEST_ITERATIONS, buffer);
  std::cout << "Printf version: " << dur << "ms\n";
  std::cout << "Converted value: " << buffer << "\n\n";

  dur = testIt(IIDToString_Fast, &testIID, TEST_ITERATIONS, buffer);
  std::cout << "Fast version: " << dur << "ms\n";
  std::cout << "Converted value: " << buffer << "\n";
}
