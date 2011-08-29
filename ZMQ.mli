type context
type socket
type file_descriptor
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

type 'a socket_option
type event_state = None | PollIn | PollOut | PollInAndPollOut

val version: unit -> int*int*int
val init: int -> context
val socket: context -> socket_type -> socket
val bind: socket -> string -> unit
val connect: socket -> string -> unit
val close: socket -> unit
val term: context -> unit

val send : ?no_block:bool -> ?send_more:bool -> socket -> string -> unit
val recv : ?no_block:bool -> socket -> string

(* OPTION MANIPULATIONS *)
val socket_type : socket_type socket_option
val linger : int socket_option
val reconnect_ivl : int socket_option
val reconnect_ivl_max : int socket_option
val backlog : int socket_option
val maxmsgsize : int64 socket_option
val swap : int64 socket_option
val rate : int64 socket_option
val recovery_ivl : int64 socket_option
val recovery_ivl_msec : int64 socket_option
val mcast_loop : bool socket_option
val hwm : int64 socket_option
val affinity : bool array socket_option
val sndbuf : int64 socket_option
val rcvbuf : int64 socket_option
val identity : string option socket_option
val subscribe : string socket_option
val unsubscribe : string socket_option

val getsockopt : socket -> 'a socket_option -> 'a
val setsockopt : socket -> 'a socket_option -> 'a -> unit

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
