
#ifndef _GSP_FRAMEWORK_PREVENT_DUPLICATE_INCLUDE_
#define _GSP_FRAMEWORK_PREVENT_DUPLICATE_INCLUDE_

//////////////////////////////
// WARNING:                 //
//////////////////////////////
//
// Use examples and external documentation to implement new modules.
//
// Do not read this file unless you are curious or doing maintenance on the framework.
//


//////////////////////////////
// INTERNAL Tools           //
//////////////////////////////

#ifdef __cplusplus
    #define C_FUNCTION__ extern "C"
#else
    #define C_FUNCTION__
#endif

// from http://newsgroups.derkeiler.com/Archive/Comp/comp.std.c/2006-01/msg00072.html
#define PP_NARG(...) \
        PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
        PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
        63,62,61,60,                   \
        59,58,57,56,55,54,53,52,51,50, \
        49,48,47,46,45,44,43,42,41,40, \
        39,38,37,36,35,34,33,32,31,30, \
        29,28,27,26,25,24,23,22,21,20, \
        19,18,17,16,15,14,13,12,11,10, \
        9,8,7,6,5,4,3,2,1,0

#define CONCAT2(a,b) a ## b
#define CONCAT(a,b) CONCAT2(a,b)
#define APPLY(M,...) M(__VA_ARGS__)
#define QUOTE_1(p) #p
#define QUOTE_2(p, ...) #p, QUOTE_1(__VA_ARGS__)
#define QUOTE_3(p, ...) #p, QUOTE_2(__VA_ARGS__)
#define QUOTE_4(p, ...) #p, QUOTE_3(__VA_ARGS__)
#define QUOTE_5(p, ...) #p, QUOTE_4(__VA_ARGS__)
#define QUOTE_6(p, ...) #p, QUOTE_5(__VA_ARGS__)
#define QUOTE_7(p, ...) #p, QUOTE_6(__VA_ARGS__)
#define QUOTE_8(p, ...) #p, QUOTE_7(__VA_ARGS__)
#define QUOTE_9(p, ...) #p, QUOTE_8(__VA_ARGS__)
#define QUOTE(...) APPLY(CONCAT(QUOTE_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

#define QUOTEnADDR_1(p) &p
#define QUOTEnADDR__2(p, ...) #p, QUOTEnADDR_1(__VA_ARGS__)
#define QUOTEnADDR_3(p, ...) &p, QUOTEnADDR__2(__VA_ARGS__)
#define QUOTEnADDR__4(p, ...) #p, QUOTEnADDR_3(__VA_ARGS__)
#define QUOTEnADDR_5(p, ...) &p, QUOTEnADDR__4(__VA_ARGS__)
#define QUOTEnADDR__6(p, ...) #p, QUOTEnADDR_5(__VA_ARGS__)
#define QUOTEnADDR_7(p, ...) &p, QUOTEnADDR__6(__VA_ARGS__)
#define QUOTEnADDR__8(p, ...) #p, QUOTEnADDR_7(__VA_ARGS__)
#define QUOTEnADDR(...) APPLY(CONCAT(QUOTEnADDR__, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

#define SEP_2(a,b) a##__v__##b
#define SEP_3(a,b,c) a##__v__##b##__v__##c
#define SEP(...) APPLY(CONCAT(SEP_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

#define SECONDS_OF_2(a, b) b
#define SECONDS_OF_4(a, b, ...) b, SECONDS_OF(__VA_ARGS__)
#define SECONDS_OF_6(a, b, ...) b, SECONDS_OF(__VA_ARGS__)
#define SECONDS_OF_8(a, b, ...) b, SECONDS_OF(__VA_ARGS__)
#define SECONDS_OF(...) APPLY(CONCAT(SECONDS_OF_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

// TODO define PAIRS_OF_3 ... such that it reports the problem
#define PAIRS_OF_2(a, b) a b
#define PAIRS_OF_4(a, b, ...) a b, PAIRS_OF(__VA_ARGS__)
#define PAIRS_OF_6(a, b, ...) a b, PAIRS_OF(__VA_ARGS__)
#define PAIRS_OF_8(a, b, ...) a b, PAIRS_OF(__VA_ARGS__)
#define PAIRS_OF(...) APPLY(CONCAT(PAIRS_OF_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

#define TYPES_AND_ADDRS_0()
#define TYPES_AND_ADDRS_1(a) typeid(a).name(), &((a)),
#define TYPES_AND_ADDRS_2(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_1(__VA_ARGS__)
#define TYPES_AND_ADDRS_3(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_2(__VA_ARGS__)
#define TYPES_AND_ADDRS_4(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_3(__VA_ARGS__)
#define TYPES_AND_ADDRS_5(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_4(__VA_ARGS__)
#define TYPES_AND_ADDRS_6(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_5(__VA_ARGS__)
#define TYPES_AND_ADDRS_7(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_6(__VA_ARGS__)
#define TYPES_AND_ADDRS_8(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_7(__VA_ARGS__)
#define TYPES_AND_ADDRS_9(a, ...) TYPES_AND_ADDRS_1(a) TYPES_AND_ADDRS_8(__VA_ARGS__)
#define TYPES_AND_ADDRS(...) APPLY(CONCAT(TYPES_AND_ADDRS_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

#define TYPES_AND_PNUMS_1(a) typeid(a).name(), &((a))
#define TYPES_AND_PNUMS_2(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_3(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_4(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_5(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_6(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_7(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_8(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS_9(a, ...) TYPES_AND_PNUMS_1(a), TYPES_AND_PNUMS(__VA_ARGS__)
#define TYPES_AND_PNUMS(...) APPLY(CONCAT(TYPES_AND_PNUMS_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)



//////////////////////////////
// INTERNAL Macros          //
//////////////////////////////
typedef void (*Framework) (const char* command, ...);

// Headers for the module writer
#define SPI_HEADERS(m, type, ...) SPI_HEADERS_##type(m, __VA_ARGS__)
#define SPI_HEADERS_param(...)
#define SPI_HEADERS_receiver(m, name, func, ...) static void func(m* m___m, PAIRS_OF(__VA_ARGS__));
#define SPI_HEADERS_emitter(m, name, func, ...)                         \
    static void func(m* m___m, PAIRS_OF(__VA_ARGS__)) {                 \
        const void *what[] = {#name, QUOTEnADDR(__VA_ARGS__), NULL};    \
        m___m->_framework("emit", what);                                \
    }
#define SPI_HEADERS_raw(...)
#define SPI_HEADERS_create(m, f) static void f(m *m___m);
#define SPI_HEADERS_init(m, f) static void f(m *m___m);
#define SPI_HEADERS_stop(m, f) static void f(m *m___m);


// Members of the generated struct used by the module writer
#define SPI_STRUCT_FIELDS(m, type, ...) SPI_STRUCT_FIELDS_##type(m, __VA_ARGS__)
#define SPI_STRUCT_FIELDS_param(m, type, name) type name;
#define SPI_STRUCT_FIELDS_receiver(...)
#define SPI_STRUCT_FIELDS_emitter(...)
#define SPI_STRUCT_FIELDS_raw(m, what) what;
#define SPI_STRUCT_FIELDS_create(...)
#define SPI_STRUCT_FIELDS_init(...)
#define SPI_STRUCT_FIELDS_stop(...)
#define CPP_SPI_STRUCT_FIELDS(m, type, ...) CPP_SPI_STRUCT_FIELDS_##type(m, __VA_ARGS__)
#define CPP_SPI_STRUCT_FIELDS_param(m, type, name) public: type name;
#define CPP_SPI_STRUCT_FIELDS_receiver(m, name, func, ...) public: void func(PAIRS_OF(__VA_ARGS__));
#define CPP_SPI_STRUCT_FIELDS_emitter(m, name, func, ...)              \
    protected: void func(PAIRS_OF(__VA_ARGS__)) {                      \
        const void *what[] = {#name, QUOTEnADDR(__VA_ARGS__), NULL};   \
        this->_framework("emit", what);                                \
    }
#define CPP_SPI_STRUCT_FIELDS_raw(m, what) public: what;
#define CPP_SPI_STRUCT_FIELDS_create(m, f) public: void f();
#define CPP_SPI_STRUCT_FIELDS_init(m, f) public: void f();
#define CPP_SPI_STRUCT_FIELDS_stop(m, f) public: void f();


// Headers and implementation of the things called by the java framework
#define EXTERNAL_HEADERS(m, type, ...) EXTERNAL_HEADERS_##type(m, __VA_ARGS__)
#define EXTERNAL_HEADERS_param(m, type, name)                        \
    C_FUNCTION__ void SEP(m,set,name)(m *m___m, type v___v) {        \
        m___m->name = v___v;                                         \
    }
#define EXTERNAL_HEADERS_receiver(m, name, func, ...)                   \
    C_FUNCTION__ void SEP(m,event,name)(m *m___m, PAIRS_OF(__VA_ARGS__)) { \
        func(m___m, SECONDS_OF(__VA_ARGS__));                           \
    }
#define EXTERNAL_HEADERS_emitter(...)
#define EXTERNAL_HEADERS_raw(...)
#define EXTERNAL_HEADERS_create(m, f) C_FUNCTION__ void SEP(m,created)(m *m___m) {f(m___m);}
#define EXTERNAL_HEADERS_init(m, f) C_FUNCTION__ void SEP(m,init)(m *m___m) {f(m___m);}
#define EXTERNAL_HEADERS_stop(m, f) C_FUNCTION__ void SEP(m,stop)(m *m___m) {f(m___m);}
#define CPP_EXTERNAL_HEADERS(m, type, ...) CPP_EXTERNAL_HEADERS_##type(m, __VA_ARGS__)
#define CPP_EXTERNAL_HEADERS_param(...) EXTERNAL_HEADERS_param(__VA_ARGS__)
#define CPP_EXTERNAL_HEADERS_receiver(m, name, func, ...)                   \
    C_FUNCTION__ void SEP(m,event,name)(m *m___m, PAIRS_OF(__VA_ARGS__)) {  \
        m___m->func(SECONDS_OF(__VA_ARGS__));                               \
    }
#define CPP_EXTERNAL_HEADERS_emitter(...)
#define CPP_EXTERNAL_HEADERS_raw(...)
#define CPP_EXTERNAL_HEADERS_create(m, f) C_FUNCTION__ void SEP(m,created)(m *m___m) {m___m->f();}
#define CPP_EXTERNAL_HEADERS_init(m, f) C_FUNCTION__ void SEP(m,init)(m *m___m) {m___m->f();}
#define CPP_EXTERNAL_HEADERS_stop(m, f) C_FUNCTION__ void SEP(m,stop)(m *m___m) {m___m->f();}


// Callback to the framework to register parameter types
#define EXTERNAL_REGISTER_PARAMETERS(m, type, ...) EXTERNAL_REGISTER_PARAMETERS_##type(m, __VA_ARGS__)
#define EXTERNAL_REGISTER_PARAMETERS_param(m, type, name) {const void *what[] = {#type, #name}; f("param", what);}
#define EXTERNAL_REGISTER_PARAMETERS_receiver(...)
#define EXTERNAL_REGISTER_PARAMETERS_emitter(...)
#define EXTERNAL_REGISTER_PARAMETERS_raw(m, what)
#define EXTERNAL_REGISTER_PARAMETERS_create(...)
#define EXTERNAL_REGISTER_PARAMETERS_init(...)
#define EXTERNAL_REGISTER_PARAMETERS_stop(...)


//#define REGISTER(type, name, ...) {const char* params[] = {QUOTE(name, __VA_ARGS__), 0}; framework(#type, params);}

//#define STRUCTURE(...) {const char* params[] = {QUOTE(__VA_ARGS__), 0}; framework("structure", params);}

//////////////////////////////
// Framework Macro          //
//////////////////////////////
#include <stdlib.h> // we might use malloc and free

#ifdef __cplusplus
#include <typeinfo>
#endif

#define BEGIN_MODULE(m)                                                 \
    typedef struct {                                                    \
        Framework _framework;                                           \
        m(SPI_STRUCT_FIELDS, m)

#define BEGIN_CPP_MODULE(m)                                             \
    class m {                                                           \
public:                                                                 \
        Framework _framework;                                           \
        m(CPP_SPI_STRUCT_FIELDS, m)


#define END_MODULE(m)                                                   \
    } m;                                                                \
    C_FUNCTION__ m* SEP(m,create)(Framework f) {                        \
        m *res = (m*) malloc(sizeof(m));                                \
        res->_framework = f;                                            \
        m(EXTERNAL_REGISTER_PARAMETERS, m)                              \
        return res;                                                     \
    }                                                                   \
    C_FUNCTION__ void SEP(m,delete)(m* m___m) {}                        \
    m(SPI_HEADERS, m)                                                   \
    m(EXTERNAL_HEADERS, m)

#define END_CPP_MODULE(m)                                               \
    };                                                                  \
    C_FUNCTION__ m* SEP(m,create)(Framework f) {                        \
        m *res = new m;                                                 \
        res->_framework = f;                                            \
        m(EXTERNAL_REGISTER_PARAMETERS, m)                              \
        return res;                                                     \
    }                                                                   \
    C_FUNCTION__ void SEP(m,delete)(m* m___m) {							\
        delete m___m;										\
    }												\
    m(CPP_EXTERNAL_HEADERS, m)


#define DECLARE_MODULE(m) BEGIN_MODULE(m) END_MODULE(m)
#define DECLARE_CPP_MODULE(m) BEGIN_CPP_MODULE(m) END_CPP_MODULE(m)

/////////////////////////////////////////////
// Framework Macro (implicit version)      //
/////////////////////////////////////////////

#define emitNamedEvent0(name) {                                     \
        const void *what[] = {name, NULL};                              \
        this->_framework("emit", what);                                 \
    }
#define emitNamedEvent(name, ...) {                                     \
        const void *what[] = {name, TYPES_AND_ADDRS(__VA_ARGS__) NULL}; \
        this->_framework("emit", what);                                 \
    }
#define CLASS_AS_MODULE(m)                                              \
    C_FUNCTION__ m* SEP(m,create)(Framework f) {                        \
        m *res = new m;                                                 \
        res->_framework = f;                                            \
        return res;                                                     \
    }

static void GSPPassiveFramework(const char* command, ...) {}
#define GSP_PASSIVE_MODULE(m) SEP(m,create)(&GSPPassiveFramework)

#ifdef __cplusplus
#define FIELD_WITH_SETTER(type,name,setName)                            \
    type name;                                                          \
    virtual void setName(type name) {this->name = name;}
#define STRING_FIELD_WITH_SETTER(name,setName)                          \
    std::string name;                                                   \
    virtual void setName(char* name) {this->name = name;}
#endif

#ifdef PASSIVE_GSP_FRAMEWORK
#undef emitNamedEvent
#define emitNamedEvent(name, ...) {}
#define GSP_CREATE(m) (SEP(m,create)(NULL))
#define GSP_DELETE(m) (SEP(m,delete)(NULL))
#endif


#endif
