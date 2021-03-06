ocaml_lib_folder = /opt/local/lib/ocaml

# Make the wrapper.
wrapper.o: wrapper.c
	ocamlc -c $<

lib_wrapper_stubs.so: wrapper.o
	ocamlmklib -o _wrapper_stubs  $<  \
		-L/usr/local/lib -lzmq
# Done making the wrapper

ZMQ.cmi: ZMQ.mli
	ocamlc -c ZMQ.mli

ZMQ.cmo: ZMQ.ml ZMQ.cmi
	ocamlc -c ZMQ.ml -I uint

ZMQ.cma: ZMQ.cmo lib_wrapper_stubs.so
	ocamlc -a -o $@  $< -dllib -l_wrapper_stubs \
            -ccopt -L/usr/local/lib  \
            -cclib -lzmq

test: test.ml ZMQ.cmo lib_wrapper_stubs.so 
	ocamlc -c test.ml
	ocamlc -o $@ ZMQ.cmo test.cmo -dllib -l_wrapper_stubs \
            -ccopt -L/usr/local/lib  \
            -cclib -lzmq

install: ZMQ.cmi ZMQ.cma  ZMQ.cmo dll_wrapper_stubs.so
	cp ZMQ.cmi ZMQ.cma  ZMQ.cmo dll_wrapper_stubs.so $(ocaml_lib_folder)

all: ZMQ.cma test

clean:
	rm -f *.[oa] *.so *.cm[ixoa] *.cmxa *.a
	-rm test

.PHONY: clean update-uint test all