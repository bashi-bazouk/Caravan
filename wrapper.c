#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/threads.h>

#include <zmq.h>

#define RAISE(name,message) caml_raise_with_string(*caml_named_value(name),message)
#define TYPEMAP(name) static int const name[]
#define Val_none Val_int(0)
#define Some_val(v) Field(v,0)

/* SOCKETS */
TYPEMAP(socket_type) = {
  ZMQ_REQ,
  ZMQ_REP,
  ZMQ_XREQ,
  ZMQ_XREP,
  ZMQ_PUB,
  ZMQ_SUB,
  ZMQ_PUSH,
  ZMQ_PULL,
  ZMQ_PAIR
};

TYPEMAP(send_flag) = {
  ZMQ_NOBLOCK,
  ZMQ_SNDMORE
};

CAMLprim value wrap_version() {
  //Declare the working variables.
  CAMLparam0();

  //Generate the version numbers.
  int version[3];
  zmq_version(version,version+1,version+2);

  //Allocate the return structure.
  CAMLlocal1(values);
  values = caml_alloc_tuple(3);
  
  //Populate the return structure.
  Store_field(values,0,Val_int(version[0]));
  Store_field(values,1,Val_int(version[1]));
  Store_field(values,2,Val_int(version[2]));

  //Call the OCaml return procedure.
  CAMLreturn(values);
}

CAMLprim value wrap_init(value io_threads) {
  //Declare the parameters.
  CAMLparam1(io_threads);

  //Generate the context.
  void *context = zmq_init(Int_val(io_threads));

  if(context){//If we have a context, then return it.
    CAMLreturn((value)context);
  }

  //Otherwise, report the error.
  switch(zmq_errno()) {
  case EINVAL:
    RAISE("EINVAL","An invalid number of io threads was requested.");
    break;
  default:
    caml_raise_with_string(*caml_named_value("ERR"),"Error in wrap_init.");
  }
}

CAMLprim value wrap_socket(value caml_context, value caml_socket_type) {
  //Declare the parameters.
  CAMLparam2(caml_context,caml_socket_type);
  void *context = (void*)caml_context;
  int type = socket_type[Int_val(caml_socket_type)];

  //Generate the socket.
  void *socket = zmq_socket(context,type);

  if(socket){//If there is a socket, then return it.
    CAMLreturn((value)socket);
  }

  //Otherwise, report the error.
  switch(zmq_errno()) {
  case EINVAL:
    RAISE("EINVAL","The requested socket type is invalid.");
  case EFAULT:
    RAISE("EFAULT","The provided context was not valid.");
  case ETERM:
    RAISE("ETERM","The context specified was terminated.");
  default:
    RAISE("ERR","Error in wrap_socket.");
  }
}

CAMLprim value wrap_bind(value caml_socket, value caml_endpoint) {
  //Declare the parameters
  CAMLparam2(caml_socket, caml_endpoint);
  void *socket = (void *)caml_socket;
  char *endpoint = String_val(caml_endpoint);

  //Perform the call
  int rc = zmq_bind(socket,endpoint);

  if(!rc){//If there is no error, then return.
    CAMLreturn(Val_unit);
  }
  //Otherwise, report errors
  
  printf("Error number: %i\n",zmq_errno());
  switch(zmq_errno()){
  case EPROTONOSUPPORT:
    RAISE("EPROTONOSUPPORT","The requested transport protocol is not supported.");
  case ENOCOMPATPROTO:
    RAISE("ENOCOMPATPROTO","The requested transport protocol is not compatible with the socket type.");
  case EADDRINUSE:
    RAISE("EADDRINUSE","The requested address is already in use.");
  case EADDRNOTAVAIL:
    RAISE("EADDRNOTAVAIL","The requested address was not local.");
  case ENODEV:
    RAISE("ENODEV","The requested address specifies a nonexistent interface.");
  case ETERM:
    RAISE("ETERM","The 0MQ context associated with the specified socket was terminated.");
  case EFAULT:
    RAISE("EFAULT","The provided socket was not valid.");
  case EMTHREAD:
    RAISE("EMTHREAD","No I/O thread is available to accomplish the task.");
  default:
    RAISE("ERR","Unknown error in wrap_bind.");
  }
}
  

CAMLprim value wrap_connect(value caml_socket, value caml_endpoint) {
  //Declare the parameters
  CAMLparam2(caml_socket, caml_endpoint);
  void *socket = (void *)caml_socket;
  char *endpoint = String_val(caml_endpoint);

  //Perform the call
  int rc = zmq_connect(socket,endpoint);

  //If all is well, return.
  if(!rc){
    CAMLreturn(Val_unit);
  }

  //Otherwise, report the error.
  switch(zmq_errno()){
  case EPROTONOSUPPORT:
    RAISE("EPROTONOSUPPORT","The requested transport protocol is not supported.");
  case ENOCOMPATPROTO:
    RAISE("ENOCOMPATPROTO","The requested transport protocol is not compatible with the socket type.");
  case ETERM:
    RAISE("ETERM","The 0MQ context associated with the specified socket was terminated.");
  case EFAULT:
    RAISE("EFAULT","The provided socket was not valid.");
  case EMTHREAD:
    RAISE("EMTHREAD","No I/O thread is available to accomplish the task.");
  default:
    RAISE("ERR","Error in wrap_zmq_connect.");
  }
}

CAMLprim value wrap_close(value caml_socket) {
  //Declare the parameters.
  CAMLparam1(caml_socket);
  void *socket = (void*)caml_socket;
  
  //Perform the call.
  int rc = zmq_close(socket);
  
  if(!rc){//If all is well, return.
    CAMLreturn(Val_unit);
  }
  
  //Otherwise, report the error.
  switch(zmq_errno()){
  case EFAULT:
    RAISE("EFAULT","The provided socket was not valid.");
  default:
    RAISE("ERR","Unknown error in wrap_close");
  }
}

CAMLprim value wrap_term(value caml_context) {
  //Declare the parameters.
  CAMLparam1(caml_context);
  void *context = (void*)caml_context;

  //Perform the call
  int rc = zmq_term(context);
  
  if(!rc){//If all is well, return.
    CAMLreturn(Val_unit);
  }
  
  //Otherwise, report the error.
  switch(zmq_errno()){
  case EFAULT:
    RAISE("EFAULT","The provided context was not valid.");
  case EINTR:
    RAISE("EINTR","Termination was interrupted by a signal.  It can be restarted if needed.");
  default:
    RAISE("ERR","Unknown error in wrap_term");
  }
}

/* SEND/RECV */

CAMLprim value wrap_send(value caml_socket, value caml_message, value caml_flags) {
  //Declare the parameters.
  CAMLparam3(caml_socket, caml_flags, caml_message);
  void *socket = (void*)caml_socket;
  
  //Generate the option flags.
  CAMLlocal1(head);
  int flags = 0;
  while (caml_flags!=Val_emptylist) {
    head = Field(caml_flags, 0);
    flags = flags | (send_flag[Int_val(head)]);
    caml_flags = Field(caml_flags, 1);
  }

  //Allocate the message.
  zmq_msg_t message;
  int rc = zmq_msg_init_size(&message, caml_string_length(caml_message));

  if(rc){
    switch(zmq_errno()) {
    case ENOMEM:
      RAISE("ENOMEM","Insufficient storage space is available.");
    default:
      RAISE("ERR","Unknown error in wrap_send during zmq_msg_init_size.");
    }
  }

  memcpy((void *)zmq_msg_data(&message), String_val(caml_message), caml_string_length(caml_message));

  //Send the message.
  caml_enter_blocking_section();
  rc = zmq_send(socket, &message, flags);
  caml_leave_blocking_section();
  
  if(!rc){//If all is well, return.
    CAMLreturn(Val_unit);
  }

  //Otherwise, report the error.
  switch(zmq_errno()) {
  case EAGAIN:
    RAISE("EAGAIN","Non-blocking mode was requested and the message cannot be sent at the moment.");
  case ENOTSUP:
    RAISE("ENOTSUP","The zmq_send() operation is not supported by this socket type.");
  case EFSM:
    RAISE("EFSM","The zmq_send() operation cannot be performed on this socket at the moment due to the socket not being in the appropriate state.");
  case ETERM:
    RAISE("ETERM","The 0MQ context associated with the specified socket was terminated.");
  case EFAULT:
    RAISE("EFAULT","The provided socket was not valid.");
  case EINTR:
    RAISE("EINTR","The operation was interrupted by delivery of a signal before the message was sent.");
  default:
    RAISE("EINTR","Error in wrap_zmq_send.");
  }
}

CAMLprim value wrap_recv(value caml_socket, value caml_flags) {
  //Declare the parameters.
  CAMLparam2(caml_socket,caml_flags);
  void *socket = (void *)caml_socket;

  //Generate the option flags.
  int flags = 0;
  if(caml_flags!=Val_emptylist && Int_val(Field(caml_flags, 0))==1) {
    printf("zmq_noblock set to true.");
    flags = ZMQ_NOBLOCK;
  }
  
  //Initialize the message.
  zmq_msg_t message;
  int rc = zmq_msg_init(&message);

  if(rc) {
    RAISE("ERR","Unknown error in wrap_recv at zmq_msg_init.");
  }

  //Receive into the message.
  caml_enter_blocking_section();
  rc = zmq_recv(socket, &message, flags);
  caml_leave_blocking_section();

  
  if(!rc){//If all is well, copy out the data and return..

    //Copy the data into a new buffer.
    size_t size = zmq_msg_size (&message);
    value caml_message = caml_alloc_string(size);
    memcpy (String_val(caml_message), zmq_msg_data (&message), size);
    rc = zmq_msg_close(&message);
    
    if(rc){
      RAISE("ERR","Unknown error in wrap_recv at zmq_msg_close.");
    }

    CAMLreturn(caml_message);
  }

  //Otherwise, report the error.
  switch(zmq_errno()){
  case EAGAIN:
    RAISE("EAGAIN","Non-blocking mode was requested and no messages are available at the moment.");
  case ENOTSUP:
    RAISE("ENOTSUP","The zmq_recv() operation is not supported by this socket type.");
  case EFSM:
    RAISE("EFSM","The zmq_recv() operation cannot be performed on this socket at the moment due to the socket not being in the appropriate state.");
  case ETERM:
    RAISE("ETERM","The 0MQ context associated with the specified socket was terminated.");
  case EFAULT:
    RAISE("EFAULT","The provided socket was not valid.");
  case EINTR:
    RAISE("EINTR","The operation was interrupted by delivery of a signal before a message was available.");
  default:
    RAISE("ERR","Unknown error in wrap_recv at zmq_recv.");
  }
}


/* OPTIONS */
TYPEMAP(option_name) = {
  ZMQ_LINGER,
  ZMQ_RECONNECT_IVL,
  ZMQ_RECONNECT_IVL_MAX,
  ZMQ_BACKLOG,
  ZMQ_SWAP,
  ZMQ_RATE,
  ZMQ_RECOVERY_IVL,
  ZMQ_RECOVERY_IVL_MSEC,
  ZMQ_HWM,
  ZMQ_SNDBUF,
  ZMQ_RCVBUF,
  ZMQ_IDENTITY,
  ZMQ_SUBSCRIBE,
  ZMQ_UNSUBSCRIBE,
  ZMQ_MCAST_LOOP,
  ZMQ_AFFINITY,
};

static size_t const map_option_len[] = {
  sizeof (int),       //   ZMQ_LINGER
  sizeof (int),       //   ZMQ_RECONNECT_IVL
  sizeof (int),       //   ZMQ_RECONNECT_IVL_MAX
  sizeof (int),       //   ZMQ_BACKLOG
  sizeof (int64),     //   ZMQ_SWAP
  sizeof (int64),     //   ZMQ_RATE
  sizeof (int64),     //   ZMQ_RECOVERY_IVL
  sizeof (int64),     //   ZMQ_RECOVERY_IVL_MSEC
  sizeof (uint64),    //   ZMQ_HWM
  sizeof (uint64),    //   ZMQ_SNDBUF
  sizeof (uint64),    //   ZMQ_RCVBUF
  256*(sizeof (char)),//   ZMQ_IDENTITY
  -1,                 //   ZMQ_SUBSCRIBE
  -1,                 //   ZMQ_UNSUBSCRIBE
  sizeof (int64),     //   ZMQ_MCAST_LOOP
  sizeof (uint64),    //   ZMQ_AFFINITY
};

CAMLprim value wrap_getsockopt(value caml_socket, value caml_option) {
  //Declare the parameters
  CAMLparam2(caml_socket, caml_option);
  void *socket = (void *)caml_socket;
  int n = Int_val(caml_option);

  //Get value
  int name = option_name[n];
  size_t option_len = map_option_len[n];
  void *option_value = malloc(option_len);

  int rc = zmq_getsockopt(socket,name,option_value,&option_len);
  
  if(!rc) {  //If all is well, return the value.
    switch(name) {
    case ZMQ_LINGER:             //int <-> int
    case ZMQ_RECONNECT_IVL:      //int <-> int
    case ZMQ_RECONNECT_IVL_MAX:  //int <-> int
    case ZMQ_BACKLOG: {          //int <-> int
      int val = *((int*)option_value);
      CAMLreturn(Val_int(val));
    }
    case ZMQ_SWAP:               //int64 <-> int64
    case ZMQ_RATE:               //int64 <-> int64
    case ZMQ_RECOVERY_IVL:       //int64 <-> int64
    case ZMQ_RECOVERY_IVL_MSEC: {//int64 <-> int64
      int64 val = *((int64*)option_value);
      CAMLreturn(caml_copy_int64(val));
    }
    case ZMQ_HWM:               //int64 <-> uint64
    case ZMQ_SNDBUF:            //int64 <-> uint64
    case ZMQ_RCVBUF: {          //int64 <-> uint64
      uint64 val = *((uint64*)option_value);
      CAMLreturn(caml_copy_int64(val));
    }
    case ZMQ_IDENTITY: {        //string option <-> string
      char *str = (char*)option_value;
      if(strcmp(str,"")){
	CAMLlocal1(val);
	val = caml_alloc(1, 0);
	Store_field(val, 0, caml_copy_string(str));
	CAMLreturn(val);
      }else{
	CAMLreturn(Val_none);
      }
    }
    case ZMQ_MCAST_LOOP:        //bool <-> int64
    case ZMQ_AFFINITY: {        //bool array <-> uint64
      uint64 val = *((uint64*)option_value);
      uint64 mask;
      value caml_val = caml_alloc(64,0);
      int i;
      for(i=0; i<64; i++){
	mask = 1 << i;
	if(val & mask){
	  Store_field(caml_val,i,Val_true);
	}else{
	  Store_field(caml_val,i,Val_false);
	}
      }
      CAMLreturn(caml_val);
    }
    default:
      RAISE("ERR","This should not be happening.");
    };
  }

  //Otherwise, report the error.
  switch(zmq_errno()) {
  case EINVAL:
    RAISE("EINVAL", //This shouldn't ever happen.
	  "The requested option option_name is unknown, or the requested option_len or option_value is invalid, or the size of the buffer pointed to by option_value, as specified by option_len, is insufficient for storing the option value.");
    break;
  case ETERM:
    RAISE("ETERM",
	  "The 0MQ context associated with the specified socket was terminated.");
    break;
  case EFAULT: 
    RAISE("ETERM",
	  "The provided socket was not valid (NULL).");
    break;
  case EINTR:
    RAISE("EINTR",
	  "The operation was interrupted by delivery of a signal.");
    break;
  default:
    RAISE("ERR","Unknown error in wrap_getsockopt.");
  }
}

CAMLprim value wrap_setsockopt(value caml_socket, value caml_option, value caml_val) {
  //Declare the parameters
  CAMLparam3(caml_socket, caml_option, caml_val);
  void *socket = (void *)caml_socket;
  int n = Int_val(caml_option);
  int name = option_name[n];

  //Perform the call
  int rc;
  switch(name) {
  case ZMQ_LINGER:             //int <-> int
  case ZMQ_RECONNECT_IVL:      //int <-> int
  case ZMQ_RECONNECT_IVL_MAX:  //int <-> int
  case ZMQ_BACKLOG: {          //int <-> int
    //On 64 bit machines, OCaml ints are 64 bits and C ints may be 32 bits.
    long long_val = Long_val(caml_val);
    int int_val;
    if(long_val > (long)INT_MAX){
      int_val = INT_MAX;
    }else{
      int_val = (int)long_val;
    }
    rc = zmq_setsockopt(socket, name, &int_val, sizeof(int_val));
    break;
  }
  case ZMQ_SWAP:               //int64 <-> int64
  case ZMQ_RATE:               //int64 <-> int64
  case ZMQ_RECOVERY_IVL:       //int64 <-> int64
  case ZMQ_RECOVERY_IVL_MSEC: {//int64 <-> int64
    int64 val = Int64_val(caml_val);
    rc = zmq_setsockopt(socket, name, (void *)&val, sizeof(int64));
    break;
  }
  case ZMQ_HWM:                //int64 <-> uint64
  case ZMQ_SNDBUF:             //int64 <-> uint64
  case ZMQ_RCVBUF: {           //int64 <-> uint64
    uint64 val = Int64_val(caml_val);
    rc = zmq_setsockopt(socket, name, (void *)&val, sizeof(val));
    break;
  }
  case ZMQ_IDENTITY:           //string option <-> string
    if(caml_val==Val_none){
      rc = zmq_setsockopt(socket, name, NULL, sizeof(NULL));
    }else{
      char *val = String_val(Some_val(caml_val));
      rc = zmq_setsockopt(socket, name, val, sizeof(val));
    }
    break;
  case ZMQ_SUBSCRIBE:          //string <-> string
  case ZMQ_UNSUBSCRIBE: {      //string <-> string
    char *val = String_val(caml_val);
    rc = zmq_setsockopt(socket, name, (void *)val, sizeof(val));
    break;
  }
  case ZMQ_MCAST_LOOP:         //bool <-> int64
  case ZMQ_AFFINITY: {         //bool array <-> uint64
    uint64 val = 0L;
    int i;
    for(i=0; i<64; i++){
      if(Bool_val(Field(caml_val,i))){
	val += 1 << i;
      }
    }
    rc = zmq_setsockopt(socket, name, (void *)&val, sizeof(val));
    break;
  }
  default:
    RAISE("ERR","This should not be happening.");
  };

  if(!rc) {//If all is well, return
    CAMLreturn(Val_unit);
  }

  //Otherwise, report errors.
  switch(zmq_errno()){
  case EINVAL:
    RAISE("EINVAL","The requested option is unknown, or the requested option_len or option_value is invalid.");
  case ETERM:
    RAISE("ETERM","The 0MQ context associated with the specified socket was terminated.");
  case EFAULT:
    RAISE("EFAULT","The provided socket was not valid.");
  case EINTR:
    RAISE("EINTR","The operation was interrupted by delivery of a signal.");
  default:
    RAISE("ERR","Unknown error in wrap_setsockopt.");
  }
}
