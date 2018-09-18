#ifndef VIF_UTILITY_ARGV_HPP
#define VIF_UTILITY_ARGV_HPP

#include <fstream>
#include "vif/core/vec.hpp"
#include "vif/core/string_conversion.hpp"
#include "vif/utility/string.hpp"

namespace vif {
    #define arg_list(...) #__VA_ARGS__, __VA_ARGS__

    namespace impl {
        template<typename T, typename U>
        bool read_args_n2T_(T& t, const U& u) {
            t = static_cast<T>(u);
            return true;
        }

        template<typename T, typename U,
            typename enable = typename std::enable_if<!std::is_same<T, std::string>::value>::type>
        bool read_args_n2T_(vec<1,T>& t, const U& u) {
            t = {static_cast<T>(u)};
            return true;
        }

        template<typename T,
            typename enable = typename std::enable_if<!std::is_same<T, vec<1,std::string>>::value>::type>
        bool read_args_n2T_(T& t, const std::string& s) {
            return from_string(s, t);
        }

        template<typename U>
        bool read_args_n2T_(vec<1,std::string>& t, const U& u) {
            t = {to_string(u)};
            return true;
        }

        template<typename U>
        bool read_args_n2T_(vec<1,std::string>& t, const std::string& u) {
            t = {u};
            return true;
        }

        template<typename U>
        bool read_args_n2T_(std::string& t, const U& u) {
            t = to_string(u);
            return true;
        }

        inline bool read_args_n2T_(std::string& t, const std::string& s) {
            t = s;
            return true;
        }

        template<typename T, typename enable = typename std::enable_if<!meta::is_vec<T>::value>::type>
        bool read_args_impl_(const std::string& arg, bool& read, bool& valid, const std::string& name, T& t) {
            auto p = arg.find_first_of('=');
            if (p == arg.npos) {
                if (arg[0] == '-') {
                    if (arg.substr(1) != name) {
                        return false;
                    }
                } else if (arg != name) {
                    return false;
                }

                read = true;
                valid = read_args_n2T_(t, 1);
            } else {
                int_t p0 = arg[0] == '-' ? 1 : 0;
                std::string aname = trim(arg.substr(p0, p - p0));
                if (aname != name) {
                    return false;
                }

                read = true;
                std::string value = trim(trim(arg.substr(p+1)), "'\"");
                valid = read_args_n2T_(t, value);
            }

            return true;
        }

        template<typename T>
        bool read_args_impl_(const std::string& arg, bool& read, bool& valid, const std::string& name, vec<1,T>& t) {
            auto p = arg.find_first_of('=');
            if (p == arg.npos) {
                if (arg[0] == '-') {
                    if (arg.substr(1) != name) {
                        return false;
                    }
                } else if (arg != name) {
                    return false;
                }

                read = true;
                valid = read_args_n2T_(t, 1);
            } else {
                int_t p0 = arg[0] == '-' ? 1 : 0;
                std::string aname = trim(arg.substr(p0, p - p0));
                if (aname != name) {
                    return false;
                }

                read = true;
                std::string value = trim(arg.substr(p+1));

                if (!value.empty() && value.front() == '[' && value.back() == ']') {
                    vec<1,meta::rtype_t<T>> tmpv;

                    value.erase(0,1); value.pop_back();

                    if (!value.empty()) {
                        vec1s vals = split(value, ",");
                        tmpv.reserve(vals.size());
                        meta::rtype_t<T> tmp;
                        for (auto& s : vals) {
                            bool v = read_args_n2T_(tmp, trim(trim(s), "'\""));
                            if (!v) {
                                valid = false;
                                return false;
                            } else {
                                tmpv.push_back(tmp);
                            }
                        }
                    }

                    t = tmpv;
                    valid = true;
                } else {
                    meta::rtype_t<T> v;
                    valid = read_args_n2T_(v, trim(trim(value), "'\""));
                    if (valid) {
                        t.clear();
                        t.push_back(v);
                    }
                }
            }

            return true;
        }

        inline void read_args_(vec1s& argv, vec1b& read, vec1b& valid, const std::string& names) {}

        template<typename T, typename ... Args>
        void read_args_(vec1s& argv, vec1b& read, vec1b& valid, const std::string& names, T& t,
            Args&& ... args) {

            std::size_t pos = names.find_first_of(',');
            std::string tname = trim(names.substr(0, pos));
            auto p = tname.find_last_of('.');
            if (p != tname.npos) {
                tname = trim(tname.substr(p+1));
            }

            vec1u idm = where(find(argv, tname) != npos);
            bool found = false;
            for (auto& i : idm) {
                if (read_args_impl_(argv[i], read[i], valid[i], tname, t)) {
                    found = true;
                }
                if (!valid[i]) {
                    warning("could not convert '", tname, "' argument to type ", pretty_type_t(T));
                    break;
                }
            }

            if (!found) {
                argv.push_back(tname+"="+to_string(t));
                read.push_back(true);
                valid.push_back(true);
            }

            if (pos != names.npos) {
                read_args_(argv, read, valid, names.substr(pos+1), std::forward<Args>(args)...);
            } else if (sizeof...(Args) != 0) {
                error("read_args: too few names provided");
                note("please use the arg_list() macro and make sure that all variables are "
                    "placed there");
                throw std::logic_error("read_args: too few names provided");
            }
        }

        template<typename T, typename ... Args>
        void read_args_(vec1s& argv, vec1b& read, vec1b& valid, const std::string& names,
            impl::named_t<T> t, Args&& ... args) {

            std::size_t pos = names.find_first_of(')');
            if (pos != names.npos) ++pos;

            vec1u idm = where(find(argv, t.name) != npos);
            bool found = false;
            for (auto& i : idm) {
                if (read_args_impl_(argv[i], read[i], valid[i], t.name, t.obj)) {
                    found = true;
                }
                if (!valid[i]) {
                    warning("could not convert '", t.name, "' argument to type ", pretty_type_t(T));
                    break;
                }
            }

            if (!found) {
                argv.push_back(t.name+"="+to_string(t.obj));
                read.push_back(true);
                valid.push_back(true);
            }

            if (pos != names.npos && pos != names.size()) {
                read_args_(argv, read, valid, names.substr(pos+1), std::forward<Args>(args)...);
            }
        }
    }

    struct program_arguments {
        program_arguments(int argc, char* argv[]) {
            if (argc <= 1) return;

            uint_t narg = argc;
            argv_.resize(narg-1);
            for (uint_t i : range(1, narg)) {
                argv_.safe[i-1] = trim(argv[i]);
            }

            read_.resize(narg-1);
            valid_ = !read_;
        }

        explicit program_arguments(const vec1s& args) {
            argv_ = args;
            read_ = replicate(false, args.size());
            valid_ = replicate(true, args.size());
        }

        ~program_arguments() {
            vec1u idm = where(!read_);
            for (auto& i : idm) {
                warning("unrecognized program argument '", argv_[i],"'");
            }
        }

        void merge(const program_arguments& pa) {
            append(argv_, pa.argv_);
            append(read_, pa.read_);
            append(valid_, pa.valid_);
        }

        template<typename ... Args>
        void read(const std::string& names, Args&& ... args) {
            impl::read_args_(argv_, read_, valid_, names, std::forward<Args>(args)...);
        }

        std::string serialize() const {
            return collapse(argv_, " ");
        }

    private :
        vec1s argv_;
        vec1b read_;
        vec1b valid_;
    };

    template<typename ... Args>
    void read_args(uint_t argc, char* argv[], const std::string& names, Args&& ... args) {
        program_arguments pa(argc, argv);
        pa.read(names, std::forward<Args>(args)...);
    }

    inline void save_args(const std::string& file, const std::string& pname, int argc, char* argv[]) {
        std::ofstream cmd(file);
        cmd << pname;
        for (int i = 1; i < argc; ++i) {
            cmd << " " << argv[i];
        }
    }

    inline std::string make_cmd(int argc, char* argv[]) {
        std::string cmd = argv[0];
        for (int i = 1; i < argc; ++i) {
            std::string tmp = argv[i];
            if (tmp.find_first_of(" \t") != tmp.npos) {
                uint_t p = tmp.find_first_of("=");
                if (p == tmp.npos) {
                    tmp = "\"" + tmp + "\"";
                } else {
                    tmp = tmp.substr(0, p+1) + "\"" + tmp.substr(p+1) + "\"";
                }
            }
            cmd += " "+tmp;
        }

        return cmd;
    }
}

#endif
