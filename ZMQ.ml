(*SOCKETS*)
type context
type socket

type socket_type =
    REQ
  | REP
  | XREQ
  | XREP
  | PUB
  | SUB
  | PUSH
  | PULL
  | PAIR

external version: unit -> int*int*int = "wrap_version"
external init: int -> context = "wrap_init"
external socket: context -> socket_type -> socket = "wrap_socket"
external bind: socket -> string -> unit = "wrap_bind"
external connect: socket -> string -> unit = "wrap_connect"
external close: socket -> unit = "wrap_close"
external term: context -> unit = "wrap_term"

(*SEND/RECV*)
type send_flag = SendNoBlock | SendMore
type recv_flag = RecvNoBlock

external send': socket -> string -> send_flag list -> unit = "wrap_send"
external recv': socket -> recv_flag list -> string = "wrap_recv"

let send ?(no_block=false) ?(send_more=false) socket string =
  let flags = if no_block then [SendNoBlock] else [] in 
  let flags = if send_more then SendMore::flags else flags in
  send' socket string flags

let recv ?(no_block=false) socket =
  let flags = if no_block then [RecvNoBlock] else [] in 
  recv' socket flags


(* OPTIONS *)
type scalar_socket_option =
  (* Int options *)
  | LINGER
  | RECONNECT_IVL
  | RECONNECT_IVL_MAX
  | BACKLOG
  (* Int64 options *)
  | SWAP
  | RATE
  | RECOVERY_IVL
  | RECOVERY_IVL_MSEC
  | HWM
  | SNDBUF
  | RCVBUF
  (* String options *)
  | IDENTITY
  | SUBSCRIBE
  | UNSUBSCRIBE
  (* Boolean options *)
  | MCAST_LOOP
  (* Boolean Array option *)
  | AFFINITY

type 'a socket_option = (unit -> socket -> 'a) * (unit -> socket -> 'a -> unit)

type event_state =
  | Neither
  | PollIn
  | PollOut
  | PollInAndPollOut

type file_descriptor

let thunk x () = x
let commute f x y = f y x

(* Most options are fine to just pass through. *)
external getsockopt'': socket -> scalar_socket_option -> 'a = "wrap_getsockopt"
external setsockopt'': socket -> scalar_socket_option -> 'a -> unit = "wrap_setsockopt"
let getsockopt' option = thunk ((commute getsockopt'') option)
let setsockopt' option = thunk ((commute setsockopt'') option)

(* However, special setters need to be checked. *)
let set_uint64 option () socket n: unit =
  let fail name = failwith ("Cannot set "^name^" to a negative number.") in
  if (Int64.compare n Int64.zero)<0 then
    (match option with
      | HWM -> fail "the high water mark"
      | SNDBUF -> fail "the transmit buffer size"
      | RCVBUF -> fail "the receive buffer size"
      | _ -> failwith "This should not be happening.")
  else
    setsockopt'' socket option n

let setsockopt_affinity () socket r: unit =
  if Array.length r <> 64 then
    failwith "Affinity must be set with an array of 64 booleans."
  else
    setsockopt' AFFINITY () socket r

let setsockopt_identity () socket opt: unit =
  match opt with 
    | Some str -> 
	if String.length str > 255 then
	  failwith "Identities cannot be longer than 255 characters."
	else
	  setsockopt' IDENTITY () socket str
    | _ -> setsockopt' IDENTITY () socket opt

(* Placeholders for getters and setters that are not part of ZMQ *)
let no_get_opt () : socket -> 'a = failwith "This option cannot be gotten."
let no_set_opt () : socket -> 'a -> unit = failwith "This option cannot be set."



let linger: int socket_option =
  (getsockopt' LINGER, setsockopt' LINGER)
let reconnect_ivl: int socket_option =
  (getsockopt' RECONNECT_IVL, setsockopt' RECONNECT_IVL)
let reconnect_ivl_max: int socket_option =
  (getsockopt' RECONNECT_IVL_MAX, setsockopt' RECONNECT_IVL_MAX)
let backlog: int socket_option =
  (getsockopt' BACKLOG, setsockopt' BACKLOG)
let swap: int64 socket_option =
  (getsockopt' SWAP, setsockopt' SWAP)
let rate: int64 socket_option =
  (getsockopt' RATE, setsockopt' RATE)
let recovery_ivl: int64 socket_option =
  (getsockopt' RECOVERY_IVL, setsockopt' RECOVERY_IVL)
let recovery_ivl_msec: int64 socket_option =
  (getsockopt' RECOVERY_IVL_MSEC, setsockopt' RECOVERY_IVL_MSEC)
let mcast_loop: bool socket_option =
  (getsockopt' MCAST_LOOP, setsockopt' MCAST_LOOP)
let hwm: int64 socket_option =
  (getsockopt' HWM, set_uint64 HWM)
let sndbuf: int64 socket_option =
  (getsockopt' SNDBUF, set_uint64 SNDBUF)
let rcvbuf: int64 socket_option =
  (getsockopt' RCVBUF, set_uint64 RCVBUF)
let identity: string option socket_option =
  (getsockopt' IDENTITY, setsockopt_identity)
let subscribe: string socket_option =
  (no_get_opt, setsockopt' SUBSCRIBE)
let unsubscribe: string socket_option =
  (no_get_opt, setsockopt' UNSUBSCRIBE)
let affinity: bool array socket_option =
  (getsockopt' AFFINITY, setsockopt_affinity)

let getsockopt (sock: socket) (option: 'a socket_option): 'a =
  (fst option) () sock

let setsockopt (sock: socket) (option: 'a socket_option): 'a -> unit =
  (snd option) () sock

(* EXCEPTIONS *)

exception AddressInUse of string
exception AddressNotAvailable of string
exception TryAgain of string
exception Fault of string
exception ErrorState of string
exception InvalidArgument of string
exception Interrupted of string
exception MessageThread of string
exception NoCompatibleProtocol of string
exception NoDevice of string
exception NoMemory of string
exception NotSupported of string
exception ProtocolNotSupported of string
exception Terminated of string
exception ZMQUnknownError of string

(* Register the exceptions for the C environment calls. *)
let _ = 
  Callback.register_exception "EADDRINUSE" (AddressInUse "any string");
  Callback.register_exception "EADDRNOTAVAIL" (AddressNotAvailable "any string");
  Callback.register_exception "EAGAIN" (TryAgain "any string");
  Callback.register_exception "EFAULT" (Fault "any string");
  Callback.register_exception "EFSM" (ErrorState "any string");
  Callback.register_exception "EINVAL" (InvalidArgument "any string");
  Callback.register_exception "EMTHREAD" (MessageThread "any string");
  Callback.register_exception "ENOCOMPATPROTO" (NoCompatibleProtocol "any string");
  Callback.register_exception "ENODEV" (NoDevice "any string");
  Callback.register_exception "ENOMEM" (NoMemory "any string");
  Callback.register_exception "ENOTSUP" (NotSupported "any string");
  Callback.register_exception "EPROTONOSUPPORT" (ProtocolNotSupported "any string");
  Callback.register_exception "ETERM" (Terminated "any string");
  Callback.register_exception "ERR" (ZMQUnknownError "any string")
