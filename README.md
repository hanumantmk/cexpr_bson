# cexpr_bson
A stab at constexpr bson creation

This basic library lays out a technique for using constexpr functions to create binary blobs at compile time.  It takes extensive advantage of c++14's extended constexpr to manage this, so you'll probably want to walk away slowly if you've got anything less than clang 3.5


###Things That Were Unnecesarily Hard

* Figuring out ieee754 double precision float encoding
* Finding a json parser that didn't need goto
* Remembering how to write C++ without any library code
* Figuring out how to write C++ without casts of any kind
* Remembering how to do all the bit magic to replace std::memcpy() with <<, >>, |, & and =
* String literals can't be used as template non-type params :(

###Things that were fantastically easy

* using a macro for constexpr made it crazy easy to debug in the runtime, then swap back to compile time once bugs were worked out.
* This works at all


#Details

##What the library invocation looks like

```c++
int main ()
{
   CEXPR_BSON_FROM_JSON(bytes, R"json(
      {
         "foo"    : "bar",
         "bar"    : "baz",
         "baz"    : 15715755,
         "neg"    : -55,
         "double" : -0.012423
      }
   )json");
   CEXPR_BSON_FROM_JSON(bytes2, R"json(
      {
         "a" : 1,
         "b" : 2,
         "c" : 3,
         "d" : {
            "key"   : "value",
            "2nd"   : 35,
            "array" : [
               1, 2, 3,
               true, false,
               0.0,
               -1152921504606846976,
               null
            ]
         }
      }
   )json");

   CEXPR_BSON_FROM_JSON(bytes3, R"json(
      {
         "only need" : "length"
      }
   )json");

   bson_iter bi(bytes.data());
   bson_iter bi2(bytes2.data());

   bi.json(std::cout);
   std::cout << std::endl;

   bi2.json(std::cout);
   std::cout << std::endl;

   std::cout << "bytes3 len: " << bytes3.len() << std::endl;

   return 0;
}
```

##Output

    $ ./cexpr_bson
    
    {"foo" : "bar", "bar" : "baz", "baz" : 15715755, "neg" : -55, "double" : -0.012423}
    {"a" : 1, "b" : 2, "c" : 3, "d" : {"key" : "value", "2nd" : 35, "array" : [1, 2, 3, true, false, 0, -1152921504606846976, null]}}
    bytes3 len: 27


From the top of main
--------------------

    $ objdump -d cexpr_bson
    
    0000000000400b40 <main>:
      400b40:       41 56                   push   %r14
      400b42:       53                      push   %rbx
      400b43:       48 83 ec 38             sub    $0x38,%rsp
      400b47:       b8 64 12 40 00          mov    $0x401264,%eax
      400b4c:       66 48 0f 6e c0          movq   %rax,%xmm0
      400b51:       0f 16 c0                movlhps %xmm0,%xmm0
      400b54:       0f 29 44 24 20          movaps %xmm0,0x20(%rsp)
      400b59:       c7 44 24 30 41 00 00    movl   $0x41,0x30(%rsp)
      400b60:       00 
      400b61:       b8 a5 12 40 00          mov    $0x4012a5,%eax
      400b66:       66 48 0f 6e c0          movq   %rax,%xmm0
      400b6b:       0f 16 c0                movlhps %xmm0,%xmm0
      400b6e:       0f 29 04 24             movaps %xmm0,(%rsp)
      400b72:       c7 44 24 10 7b 00 00    movl   $0x7b,0x10(%rsp)
      400b79:       00 
      400b7a:       48 8d 7c 24 20          lea    0x20(%rsp),%rdi
      400b7f:       be c0 17 60 00          mov    $0x6017c0,%esi
      400b84:       31 d2                   xor    %edx,%edx
      400b86:       e8 35 01 00 00          callq  400cc0 <_ZNK9bson_iter4jsonERSob>


##The read only section

    $ objdump -s -j .rodata cexpr_bson
    
    cexpr_bson:     file format elf64-x86-64
    
    Contents of section .rodata:
     401260 01000200 41000000 02666f6f 00040000  ....A....foo....
     401270 00626172 00026261 72000400 00006261  .bar..bar.....ba
     401280 7a001062 617a00ab cdef0010 6e656700  z..baz......neg.
     401290 c9ffffff 01646f75 626c6500 d5e3bed5  .....double.....
     4012a0 3a7189bf 007b0000 00106100 01000000  :q...{....a.....
     4012b0 10620002 00000010 63000300 00000364  .b......c......d
     4012c0 005e0000 00026b65 79000600 00007661  .^....key.....va
     4012d0 6c756500 10326e64 00230000 00046172  lue..2nd.#....ar
     4012e0 72617900 3a000000 10000100 00001031  ray.:..........1
     4012f0 00020000 00103200 03000000 08330001  ......2......3..
     401300 08340000 01350000 00000000 00000012  .4...5..........
     401310 36000000 00000000 00f00a37 00000000  6..........7....
     401320 62797465 7333206c 656e3a20 005b007b  bytes3 len: .[.{
     401330 002c2000 22002220 3a200074 72756500  ., ."." : .true.
     401340 66616c73 65006e75 6c6c005d 007d00    false.null.].}. 

