#ifndef STRING_HPP
#define STRING_HPP

#include "phypp/vec.hpp"
#include "phypp/reflex.hpp"
#include "phypp/range.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <regex.h>

namespace std {
    template<typename O, typename T, std::size_t N>
    O& operator << (O& o, const std::array<T,N>& v) {
        o << "{";
        for (uint_t i : range(N)) {
            if (i != 0) o << ", ";
            o << v[i];
        }
        o << "}";

        return o;
    }
}

template<typename T>
struct named_t {
    T&          obj;
    std::string name;
};

template<typename T>
named_t<T> name(T& obj, const std::string& str) {
    return {obj, str};
}

template<typename T>
std::string strn(const T& t) {
    std::ostringstream ss;
    ss << reflex::wrap(t);
    return ss.str();
}

std::string strn(const double& t) {
    std::ostringstream ss;
    ss.precision(12);
    ss << t;
    return ss.str();
}

template<std::size_t Dim, typename Type>
vec<Dim,std::string> strna(const vec<Dim,Type>& v) {
    vec<Dim,std::string> s = strarr(v.dims);
    for (uint_t i : range(v)) {
        s.data[i] = strn(v.safe[i]);
    }

    return s;
}

template<typename T>
std::string strn_sci(const T& t) {
    std::ostringstream ss;
    ss << std::scientific << reflex::wrap(t);
    return ss.str();
}


template<std::size_t Dim, typename Type>
vec<Dim,std::string> strna_sci(const vec<Dim,Type>& v) {
    vec<Dim,std::string> s = strarr(v.dims);
    for (uint_t i : range(v)) {
        s.data[i] = strn_sci(v.safe[i]);
    }

    return s;
}

template<typename T>
bool from_string(const std::string& s, T& t) {
    std::istringstream ss(s);
    ss >> t;

    if (!ss.fail()) {
        if (ss.eof()) return true;
        std::string rem;
        ss >> rem;
        return rem.find_first_not_of(" \t") == rem.npos;
    } else {
        return false;
    }
}

template<std::size_t Dim, typename T>
vec<Dim,bool> from_string(const vec<Dim,std::string>& s, vec<Dim,T>& t) {
    vec<Dim,bool> res(s.dims);
    t.resize(s.dims);
    for (uint_t i : range(s)) {
        res.safe[i] = from_string(s.safe[i], t.safe[i]);
    }

    return res;
}

template<std::size_t Dim, typename T>
vec<Dim,bool> from_string(const vec<Dim,std::string*>& s, vec<Dim,T>& t) {
    vec<Dim,bool> res(s.dims);
    t.resize(s.dims);
    for (uint_t i : range(s)) {
        res.safe[i] = from_string(s.safe[i], t.safe[i]);
    }

    return res;
}

std::string trim(std::string s, const std::string& chars = " \t") {
    std::size_t spos = s.find_first_of(chars);
    if (spos == 0) {
        std::size_t epos = s.find_first_not_of(chars);
        if (epos == s.npos) return "";
        s = s.substr(epos);
    }

    spos = s.find_last_of(chars);
    if (spos == s.size()-1) {
        std::size_t epos = s.find_last_not_of(chars);
        s = s.erase(epos+1, s.size() - epos+1);
    }

    return s;
}

std::string toupper(std::string s) {
    for (auto& c : s) {
        c = ::toupper(c);
    }
    return s;
}

std::string tolower(std::string s) {
    for (auto& c : s) {
        c = ::tolower(c);
    }
    return s;
}

std::string replace(std::string s, const std::string& pattern, const std::string& rep) {
    auto p = s.find(pattern);
    while (p != s.npos) {
        s.replace(p, pattern.size(), rep);
        p = s.find(pattern, p+rep.size());
    }

    return s;
}

bool empty(const std::string& s) {
    return s.empty();
}

uint_t distance(const std::string& t, const std::string& u) {
    uint_t n = std::min(t.size(), u.size());
    uint_t d = abs(t.size() - u.size());
    for (uint_t i : range(n)) {
        if (t[i] != u[i]) ++d;
    }

    return d;
}

uint_t find(const std::string& ts, const std::string& pattern) {
    auto p = ts.find(pattern);
    if (p != ts.npos) {
        return p;
    } else {
        return npos;
    }
}

std::string regex_get_error_(int status) {
    switch (status) {
    case REG_NOMATCH :  return "no match";
    case REG_BADPAT :   return "invalid regular expression";
    case REG_ECOLLATE : return "invalid collating element referenced";
    case REG_ECTYPE :   return "invalid character class type referenced";
    case REG_EESCAPE :  return "trailing \\ in pattern";
    case REG_ESUBREG :  return "number in \\digit invalid or in error";
    case REG_EBRACK :   return "[ ] imbalance";
    case REG_EPAREN :   return "\\( \\) or ( ) imbalance";
    case REG_EBRACE :   return "\\{ \\} imbalance";
    case REG_BADBR :    return "content of \\{ \\} invalid: not a number, number too large, "
        "more than two numbers, first larger than second";
    case REG_ERANGE :   return "invalid endpoint in range expression";
    case REG_ESPACE :   return "out of memory";
    case REG_BADRPT :   return "?, * or + not preceded by valid regular expression";
    case REG_ENOSYS :   return "the implementation does not support the function";
    default :           return "unknown error";
    }
}

void build_regex_(const std::string& regex, regex_t& re, int flags) {
    int status = regcomp(&re, regex.c_str(), flags);
    phypp_check(status == 0, "parsing regex '", regex, "': ", regex_get_error_(status));
}

bool match_(const std::string& ts, regex_t& re) {
    return regexec(&re, ts.c_str(), std::size_t(0), nullptr, 0) == 0;
}

bool match(const std::string& ts, const std::string& regex) {
    regex_t re;
    build_regex_(regex, re, REG_EXTENDED | REG_NOSUB);
    bool ret = match_(ts, re);
    regfree(&re);
    return ret;
}

template<std::size_t Dim, typename Type, typename enable = typename std::enable_if<
    std::is_same<typename std::remove_pointer<Type>::type, std::string>::value>::type>
vec<Dim,bool> match(const vec<Dim,Type>& v, const std::string& regex) {
    regex_t re;
    build_regex_(regex, re, REG_EXTENDED | REG_NOSUB);
    vec<Dim,bool> r(v.dims);
    for (uint_t i = 0; i < v.size(); ++i) {
        r.safe[i] = match_(v.safe[i], re);
    }
    regfree(&re);
    return r;
}

bool match_any_of(const std::string& ts, const vec1s& regex) {
    for (uint_t i : range(regex)) {
        regex_t re;
        build_regex_(regex.safe[i], re, REG_EXTENDED | REG_NOSUB);
        bool ret = match_(ts, re);
        regfree(&re);
        if (ret) return true;
    }

    return false;
}

vec2s extract(const std::string& ts, const std::string& regex) {
    vec2s ret;

    regex_t re;
    build_regex_(regex, re, REG_EXTENDED);

    uint_t nmatch = 0;
    uint_t p = regex.find_first_of('(');
    while (p != npos) {
        if (p == 0 || regex[p-1] != '\\') {
            ++nmatch;
        }

        p = regex.find_first_of('(', p+1);
    }

    if (nmatch == 0) return ret;

    std::vector<regmatch_t> m(nmatch+1);
    uint_t offset = 0;
    int status = regexec(&re, ts.c_str(), nmatch+1, m.data(), 0);

    while (status == 0) {
        vec1s tret;
        tret.reserve(nmatch);
        for (uint_t i : range(nmatch)) {
            tret.push_back(ts.substr(offset+m[i+1].rm_so, m[i+1].rm_eo - m[i+1].rm_so));
        }

        append<0>(ret, reform(std::move(tret), 1, nmatch));

        offset += m[0].rm_eo;
        status = regexec(&re, ts.c_str() + offset, nmatch+1, m.data(), 0);
    }

    return ret;
}

uint_t length(const std::string& s) {
    return s.size();
}

template<typename T, typename enable =
    typename std::enable_if<std::is_arithmetic<T>::value>::type>
uint_t length(T c) {
    return 1u;
}

std::string align_left(std::string s, uint_t width, char fill = ' ') {
    if (s.size() < width) {
        s += std::string(width-s.size(), fill);
    }

    return s;
}

std::string align_right(std::string s, uint_t width, char fill = ' ') {
    if (s.size() < width) {
        s.insert(0, std::string(width-s.size(), fill));
    }

    return s;
}

std::string align_center(std::string s, uint_t width, char fill = ' ') {
    if (s.size() < width) {
        uint_t n1 = (width-s.size())/2, n2 = width-s.size() - n1;
        s.insert(0, std::string(n1, fill));
        s += std::string(n2, fill);
    }

    return s;
}

bool start_with(const std::string& s, const std::string& pattern) {
    if (s.size() < pattern.size()) return false;
    for (uint_t i = 0; i < pattern.size(); ++i) {
        if (s[i] != pattern[i]) return false;
    }

    return true;
}

bool end_with(const std::string& s, const std::string& pattern) {
    if (s.size() < pattern.size()) return false;
    for (uint_t i = 1; i <= pattern.size(); ++i) {
        if (s[s.size()-i] != pattern[pattern.size()-i]) return false;
    }

    return true;
}

std::string erase_begin(std::string s, uint_t n) {
    if (n >= s.size()) {
        s.clear();
    } else {
        s.erase(0, n);
    }

    return s;
}

std::string erase_end(std::string s, uint_t n) {
    if (n >= s.size()) {
        s.clear();
    } else {
        s.erase(s.size()-n, n);
    }

    return s;
}

std::string erase_begin(std::string s, const std::string& pattern) {
    phypp_check(start_with(s, pattern), "unexpected string content: '"+s+"', "
        "should start with '"+pattern+"'");
    s.erase(0, pattern.size());
    return s;
}

std::string erase_end(std::string s, const std::string& pattern) {
    phypp_check(end_with(s, pattern), "unexpected string content: '"+s+"', "
        "should end with '"+pattern+"'");
    s.erase(s.size()-pattern.size(), pattern.size());
    return s;
}

std::string keep_start(std::string s, uint_t n = 1) {
    if (s.size() > n) {
        s.erase(n);
    }

    return s;
}

std::string keep_end(std::string s, uint_t n = 1) {
    if (s.size() > n) {
        s.erase(0, s.size()-n);
    }

    return s;
}

std::string remove_extension(std::string s) {
    auto p = s.find_last_of('.');
    if (p == s.npos) return s;
    return s.substr(0u, p);
}

// Note: all the code within this namespace comes from smallsha1,
// a lightweight SHA-1 implementation in C++. It was slightly adapted
// to fit the needs of the phy++ library.
// Source:
// http://code.google.com/p/smallsha1/
namespace sha1 {
    namespace {
        // Rotate an integer value to left.
        inline const unsigned int rol(const unsigned int value,
                const unsigned int steps) {
            return ((value << steps) | (value >> (32 - steps)));
        }

        // Sets the first 16 integers in the buffert to zero.
        // Used for clearing the W buffert.
        inline void clearWBuffert(unsigned int* buffert) {
            for (int pos = 16; --pos >= 0;)
            {
                buffert[pos] = 0;
            }
        }

        void innerHash(unsigned int* result, unsigned int* w) {
            unsigned int a = result[0];
            unsigned int b = result[1];
            unsigned int c = result[2];
            unsigned int d = result[3];
            unsigned int e = result[4];

            int round = 0;

            #define sha1macro(func,val) \
                        { \
                const unsigned int t = rol(a, 5) + (func) + e + val + w[round]; \
                                e = d; \
                                d = c; \
                                c = rol(b, 30); \
                                b = a; \
                                a = t; \
                        }

            while (round < 16) {
                sha1macro((b & c) | (~b & d), 0x5a827999)
                ++round;
            }

            while (round < 20) {
                w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
                sha1macro((b & c) | (~b & d), 0x5a827999)
                ++round;
            }

            while (round < 40) {
                w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
                sha1macro(b ^ c ^ d, 0x6ed9eba1)
                ++round;
            }

            while (round < 60) {
                w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
                sha1macro((b & c) | (b & d) | (c & d), 0x8f1bbcdc)
                ++round;
            }

            while (round < 80) {
                w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
                sha1macro(b ^ c ^ d, 0xca62c1d6)
                ++round;
            }

            #undef sha1macro

            result[0] += a;
            result[1] += b;
            result[2] += c;
            result[3] += d;
            result[4] += e;
        }
    }

    void calc(const unsigned char* sarray, const int bytelength, std::string& hexhash) {
        // Init the result array.
        unsigned int result[5] = {
            0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0
        };

        // The reusable round buffer
        unsigned int w[80];

        // Loop through all complete 64byte blocks.
        const int endOfFullBlocks = bytelength - 64;
        int endCurrentBlock;
        int currentBlock = 0;

        while (currentBlock <= endOfFullBlocks) {
            endCurrentBlock = currentBlock + 64;

            // Init the round buffer with the 64 byte block data.
            for (int roundPos = 0; currentBlock < endCurrentBlock; currentBlock += 4) {
                // This line will swap endian on big endian and keep endian
                // on little endian.
                w[roundPos++] = (unsigned int) sarray[currentBlock + 3]
                        | (((unsigned int) sarray[currentBlock + 2]) << 8)
                        | (((unsigned int) sarray[currentBlock + 1]) << 16)
                        | (((unsigned int) sarray[currentBlock]) << 24);
            }

            innerHash(result, w);
        }

        // Handle the last and not full 64 byte block if existing.
        endCurrentBlock = bytelength - currentBlock;
        clearWBuffert(w);

        int lastBlockBytes = 0;
        for (; lastBlockBytes < endCurrentBlock; ++lastBlockBytes) {
            w[lastBlockBytes >> 2] |=
                (unsigned int)sarray[lastBlockBytes + currentBlock] <<
                ((3 - (lastBlockBytes & 3)) << 3);
        }

        w[lastBlockBytes >> 2] |= 0x80 << ((3 - (lastBlockBytes & 3)) << 3);

        if (endCurrentBlock >= 56) {
            innerHash(result, w);
            clearWBuffert(w);
        }

        w[15] = bytelength << 3;
        innerHash(result, w);

        // Store hash in result pointer, and make sure we get in in the
        // correct order on both endian models.
        unsigned char hash[20];
        for (int hashByte = 20; --hashByte >= 0;) {
            hash[hashByte] =
                (result[hashByte >> 2] >> (((3 - hashByte) & 0x3) << 3)) & 0xff;
        }

        const char hexDigits[] = {"0123456789abcdef"};

        hexhash.resize(40);
        for (int hashByte = 20; --hashByte >= 0;) {
            hexhash[hashByte << 1] = hexDigits[(hash[hashByte] >> 4) & 0xf];
            hexhash[(hashByte << 1) + 1] = hexDigits[hash[hashByte] & 0xf];
        }
    }
}

std::string hash_(std::string s) {
    std::string out;
    sha1::calc(reinterpret_cast<const unsigned char*>(s.c_str()), s.size(), out);
    return out;
}

template<typename T, typename ... Args>
std::string hash_(std::string s, T&& t, Args&& ... args) {
    return hash_(s+pretty_type(t)+strn(t), std::forward<Args>(args)...);
}

template<typename ... Args>
std::string hash(Args&& ... args) {
    return hash_(std::string(), std::forward<Args>(args)...);
}

#define VECTORIZE(name) \
    template<std::size_t Dim, typename Type, typename ... Args, \
        typename enable = typename std::enable_if< \
            std::is_same<typename std::remove_pointer<Type>::type, std::string>::value>::type> \
    auto name(const vec<Dim,Type>& v, const Args& ... args) -> \
        vec<Dim,decltype(name(v[0], args...))> { \
        using ntype = decltype(name(v[0], args...)); \
        vec<Dim,ntype> r = arr<ntype>(v.dims); \
        for (uint_t i = 0; i < v.size(); ++i) { \
            r.safe[i] = name(v.safe[i], args...); \
        } \
        return r; \
    }

VECTORIZE(trim)
VECTORIZE(toupper)
VECTORIZE(tolower)
VECTORIZE(erase_begin)
VECTORIZE(erase_end)
VECTORIZE(empty)
VECTORIZE(distance)
VECTORIZE(find)
VECTORIZE(replace)
VECTORIZE(length)
VECTORIZE(align_left)
VECTORIZE(align_right)
VECTORIZE(align_center)
VECTORIZE(start_with)
VECTORIZE(end_with)
VECTORIZE(keep_start)
VECTORIZE(keep_end)

#undef VECTORIZE

template<typename T>
vec1s split(const std::string& ts, const T& pattern) {
    vec1s ret;
    std::size_t p = 0, op = 0;
    while ((p = ts.find(pattern, op)) != ts.npos) {
        ret.data.push_back(ts.substr(op, p - op));
        op = p + length(pattern);
    }

    ret.data.push_back(ts.substr(op));
    ret.dims[0] = ret.size();

    return ret;
}

vec1s cut(const std::string& ts, uint_t size) {
    uint_t ncut = floor(ts.size()/float(size));
    vec1s res(ncut);
    for (uint_t i = 0; i < ncut; ++i) {
        res.safe[i] = ts.substr(i*size, std::min(size, ts.size() - i*size));
    }

    return res;
}

vec1s wrap(const std::string& ts, uint_t width, const std::string& indent = "", bool ellipse = false) {
    vec1s ret;
    std::string s = ts;
    uint_t twidth = width;
    std::string header = "";
    while (s.size() > twidth) {
        uint_t i = twidth;
        while (i != npos && s[i] != ' ') --i;
        if (i == npos) {
            if (ellipse) {
                ret.push_back(header+s.substr(0, twidth-3)+"...");
                i = twidth+1;
                while (i < s.size() && s[i] != ' ') ++i;
                s.erase(0, i);
                s = trim(s);
            } else {
                i = twidth+1;
                while (i < s.size() && s[i] != ' ') ++i;
                ret.push_back(header+s.substr(0, i));
                s.erase(0, i);
                s = trim(s);
            }
        } else {
            ret.push_back(header+s.substr(0, i));
            s.erase(0, i);
            s = trim(s);
        }

        twidth = width - indent.size();
        header = indent;
    }

    if (!s.empty()) {
        ret.push_back(header+s);
    }

    return ret;
}

namespace format {
    void header(const std::string& msg) {
        vec1s w = wrap("  "+msg, 80, "  ");
        for (auto& s : w) {
            print(s);
        }
    }

    void paragraph(const std::string& msg) {
        vec1s w = wrap("  "+msg, 80, "  ");
        for (auto& s : w) {
            print(s);
        }
        print("");
    }

    void bullet(const std::string& name, const std::string& desc) {
        std::string header = "    "+name+": ";
        vec1s w = wrap(header+desc, 80, std::string(header.size(), ' '));
        for (auto& s : w) {
            print(s);
        }
    }

    void item(const std::string& msg) {
        std::string header = " - ";
        vec1s w = wrap(header+msg, 80, std::string(header.size(), ' '));
        for (auto& s : w) {
            print(s);
        }
    }
}

template<std::size_t Dim, typename Type, typename enable = typename std::enable_if<
    std::is_same<typename std::remove_pointer<Type>::type, std::string>::value>::type>
std::string collapse(const vec<Dim,Type>& v) {
    std::string r;
    for (auto& s : v) {
        r += s;
    }

    return r;
}

template<std::size_t Dim, typename Type, typename enable = typename std::enable_if<
    std::is_same<typename std::remove_pointer<Type>::type, std::string>::value>::type>
std::string collapse(const vec<Dim,Type>& v, const std::string& sep) {
    std::string r;
    bool first = true;
    for (auto& s : v) {
        if (!first) r += sep;
        r += s;
        first = false;
    }

    return r;
}

std::string system_var(const std::string& name, const std::string& def = "") {
    char* v = getenv(name.c_str());
    if (!v) return def;
    return v;
}

#endif
