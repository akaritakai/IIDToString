// IIDToString.cpp :
//   Testing two ways to convert an IID to a string, as described in Dave Plummer's YouTube video here:
//   https://www.youtube.com/watch?v=VYTF4KIF2z0
//

#include <iostream>
#include <combaseapi.h>

auto constexpr BUFFER_LEN = 64;
auto constexpr TEST_ITERATIONS = 10000000;

char hexLookup[65536][4];

void createHexLookup() {
  static const char hex[17] = "0123456789abcdef";

  for (int i = 0; i < 65536; i++) {
    hexLookup[i][0] = hex[i >> 12];
    hexLookup[i][1] = hex[(i >> 8) & 0xF];
    hexLookup[i][2] = hex[(i >> 4) & 0xF];
    hexLookup[i][3] = hex[i & 0xF];
  }
}

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

// Faster function to convert an IID to a string representation using lookup tables etc.
// This version uses a 256-entry lookup table for the hex values.
// Turns out not to be any faster than the 16-entry version.
void IIDToString_Fast2(const IID* iid, char* out)
{
  static const unsigned char order[16] = {
      3,2,1,0, 5,4, 7,6, 8,9,10,11,12,13,14,15
  };

  static const char* hex[256] = {
      "00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f",
      "10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f",
      "20","21","22","23","24","25","26","27","28","29","2a","2b","2c","2d","2e","2f",
      "30","31","32","33","34","35","36","37","38","39","3a","3b","3c","3d","3e","3f",
      "40","41","42","43","44","45","46","47","48","49","4a","4b","4c","4d","4e","4f",
      "50","51","52","53","54","55","56","57","58","59","5a","5b","5c","5d","5e","5f",
      "60","61","62","63","64","65","66","67","68","69","6a","6b","6c","6d","6e","6f",
      "70","71","72","73","74","75","76","77","78","79","7a","7b","7c","7d","7e","7f",
      "80","81","82","83","84","85","86","87","88","89","8a","8b","8c","8d","8e","8f",
      "90","91","92","93","94","95","96","97","98","99","9a","9b","9c","9d","9e","9f",
      "a0","a1","a2","a3","a4","a5","a6","a7","a8","a9","aa","ab","ac","ad","ae","af",
      "b0","b1","b2","b3","b4","b5","b6","b7","b8","b9","ba","bb","bc","bd","be","bf",
      "c0","c1","c2","c3","c4","c5","c6","c7","c8","c9","ca","cb","cc","cd","ce","cf",
      "d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","da","db","dc","dd","de","df",
      "e0","e1","e2","e3","e4","e5","e6","e7","e8","e9","ea","eb","ec","ed","ee","ef",
      "f0","f1","f2","f3","f4","f5","f6","f7","f8","f9","fa","fb","fc","fd","fe","ff"
  };

  const unsigned char* p = (const unsigned char*)iid;
  int i, j = 0;

  for (i = 0; i < 16; i++)
  {
    unsigned char b = p[order[i]];
    const char* hexStr = hex[b];

    out[j++] = hexStr[0];
    out[j++] = hexStr[1];

    // This version makes no noticeable difference in performance...
    //out[j++] = *(hexStr++);
    //out[j++] = *hexStr;

    if (j == 8 || j == 13 || j == 18 || j == 23)
      out[j++] = '-';
  }

  out[j] = '\0';
}

// Faster function to convert an IID to a string representation using lookup tables etc.
// This version uses a 64k-entry lookup table for the hex values allowing indexing
// with an uint16_t instead of a char. Note the need to switch ordering halfway through.
// This is a bit (~8%) faster than the 16-entry version.
void IIDToString_Fast3(const IID* iid, char* out)
{
  static const unsigned int order[8] = {
      1,0, 2, 3, 4,5,6,7
  };

  const uint16_t* p = (const uint16_t*)iid;
  int i, j = 0;
  int pairOrder = 0;

  for (i = 0; i < 8; i++)
  {
    const uint16_t b = p[order[i]];
    const char* hexStr = hexLookup[b];

    if (pairOrder == 0) {
      out[j++] = hexStr[0];
      out[j++] = hexStr[1];
      out[j++] = hexStr[2];
      out[j++] = hexStr[3];
    }
    else {
      out[j++] = hexStr[2];
      out[j++] = hexStr[3];
      out[j++] = hexStr[0];
      out[j++] = hexStr[1];
    }

    if (j == 8 || j == 13 || j == 23) {
      out[j++] = '-';
    } else if (j == 18) {
      out[j++] = '-';
      pairOrder = 1;
    }
  }

  out[j] = '\0';
}

// Function taking a pointer to a suitable IIDToString function and testing it with a large number
// of iterations to measure performance. Returns the time taken in milliseconds.
ULONGLONG testIt(void (*func)(const IID* iid, char* out), const IID* iid, int iterations, char* buffer) {
  strcpy_s(buffer, 64, "-empty-");
  auto start = GetTickCount64();
  for (int i = 0; i < iterations; i++) func(iid, buffer);
  return GetTickCount64() - start;
}

int main()
{
  IID testIID;
  HRESULT res;

  // Initialise the hex lookup table for the Fast3 version.
  createHexLookup();

  // Random test IID pulled from my registry.
  res = IIDFromString(L"{cecec95a-d894-491a-bee3-5e106fb59f2d}", &testIID);

  if (FAILED(res)) {
    std::cerr << "Failed to create IID from string. Error code: " << res << "\n";
    return 1;
  }

  std::cout << "Test starting...\n\n";

  char buffer[BUFFER_LEN] = { 0 };

  auto dur = testIt(IIDToString_Printf, &testIID, TEST_ITERATIONS, buffer);
  std::cout << "Printf version: " << dur << "ms\n";
  std::cout << "Converted value: " << buffer << "\n\n";

  dur = testIt(IIDToString_Fast, &testIID, TEST_ITERATIONS, buffer);
  std::cout << "Fast version: " << dur << "ms\n";
  std::cout << "Converted value: " << buffer << "\n\n";

  dur = testIt(IIDToString_Fast2, &testIID, TEST_ITERATIONS, buffer);
  std::cout << "Fast2 version: " << dur << "ms\n";
  std::cout << "Converted value: " << buffer << "\n\n";

  dur = testIt(IIDToString_Fast3, &testIID, TEST_ITERATIONS, buffer);
  std::cout << "Fast3 version: " << dur << "ms\n";
  std::cout << "Converted value: " << buffer << "\n\n";
}
