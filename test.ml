let _ = print_endline "Opening ZMQ."

open ZMQ

let _ = print_endline "Making a context."

let ctxt = init 3

let _ = print_endline "Making a socket."

let sock = socket ctxt PULL

let _ = print_endline "Getting socket options.\n"

type opts = 
    { val_linger : int;
      val_reconnect_interval : int;
      val_reconnect_ivl_max : int;
      val_backlog : int;
      val_swap : int64;
      val_rate : int64;
      val_recovery_ivl : int64;
      val_recovery_ivl_msec : int64;
      val_mcast_loop : bool;
      val_hwm : int64;
      val_sndbuf : int64;
      val_rcvbuf : int64;
    }

let get_options () =
  let get opt = getsockopt sock opt in
  let val_linger = get linger in
  print_endline ("Linger: "^(string_of_int val_linger));
  let val_reconnect_interval = get reconnect_ivl in
  print_endline ("Reconnect Interval: "^(string_of_int val_reconnect_interval));
  let val_reconnect_ivl_max = get reconnect_ivl_max in
  print_endline ("Maximum Reconnect Interval: "^(string_of_int val_reconnect_ivl_max));
  let val_backlog = get backlog in
  print_endline ("Backlog: "^(string_of_int val_backlog));
  let val_swap = get swap in
  print_endline ("Disk Offload Size: "^(Int64.to_string val_swap));
  let val_rate = get rate in
  print_endline ("Multicast Data Rate: "^(Int64.to_string val_rate));
  let val_recovery_ivl = get recovery_ivl in
  print_endline ("Multicast Recovery Interval: "^(Int64.to_string val_recovery_ivl));
  let val_recovery_ivl_msec = get recovery_ivl_msec in
  print_endline 
    ("Multicast Recovery Interval (milliseconds): "^(Int64.to_string val_recovery_ivl_msec));
  let val_mcast_loop = get mcast_loop in
  print_endline ("Allows Multicast Loopback: "^(string_of_bool val_mcast_loop));
  let val_hwm = get hwm in
  print_endline ("High Water Mark: "^(Int64.to_string val_hwm));
  let val_sndbuf = get sndbuf in
  print_endline ("Kernel Transmit Buffer Size: "^(Int64.to_string val_sndbuf));
  let val_rcvbuf = get rcvbuf in
  print_endline ("Kernel Receive Buffer Size: "^(Int64.to_string val_rcvbuf)^"\n");
  { val_linger = val_linger;
    val_reconnect_interval = val_reconnect_interval;
    val_reconnect_ivl_max = val_reconnect_ivl_max;
    val_backlog = val_backlog;
    val_swap = val_swap;
    val_rate = val_rate;
    val_recovery_ivl = val_recovery_ivl;
    val_recovery_ivl_msec = val_recovery_ivl_msec;
    val_mcast_loop = val_mcast_loop;
    val_hwm = val_hwm;
    val_sndbuf = val_sndbuf;
    val_rcvbuf = val_rcvbuf;
  }

let change_options o = 
  let set opt value = setsockopt sock opt value in
  print_endline ("Incrementing linger.");
  set linger (o.val_linger+1);
  print_endline ("Incrementing reconnect interval.");
  set reconnect_ivl (o.val_reconnect_interval+1);
  print_endline ("Incrementing maximum reconnect interval.");
  set reconnect_ivl_max (o.val_reconnect_ivl_max+1);
  print_endline ("Incrementing backlog.");
  set backlog (o.val_backlog+1);
  print_endline ("Incrementing disk offload size.");
  set swap (Int64.succ o.val_swap);
  print_endline ("Incrementing multicast data rate.");
  set rate (Int64.succ o.val_rate);
  print_endline ("Incrementing multicast recovery interval (milliseconds).");
  set recovery_ivl_msec (Int64.succ o.val_recovery_ivl_msec);
  print_endline ("Switching multicast loopback.");
  set mcast_loop (not o.val_mcast_loop);
  print_endline ("Incrementing high water mark.");
  set hwm (Int64.succ o.val_hwm);
  print_endline ("Incrementing kernel transmit buffer size.");
  set sndbuf (Int64.succ o.val_sndbuf);
  print_endline ("Incrementing kernel receive buffer size.\n");
  set rcvbuf (Int64.succ o.val_rcvbuf);
  get_options ()

let maximize_options () = 
  let set opt value = setsockopt sock opt value in
  print_endline ("Maximizing linger.");
  set linger (max_int);
  print_endline ("Maximizing reconnect interval.");
  set reconnect_ivl (max_int);
  print_endline ("Maximizing maximum reconnect interval.");
  set reconnect_ivl_max (max_int);
  print_endline ("Maximizing backlog.");
  set backlog (max_int);
  print_endline ("Maximizing disk offload size.");
  set swap (Int64.max_int);
  print_endline ("Maximizing multicast data rate.");
  set rate (Int64.max_int);
  print_endline ("Maximizing multicast recovery interval.");
  set recovery_ivl (Int64.max_int);
  print_endline ("Maximizing multicast recovery interval (msec).");
  set recovery_ivl_msec (Int64.max_int);
  print_endline ("Maximizing high water mark.");
  set hwm (Int64.max_int);
  print_endline ("Maximizing kernel transmit buffer size.");
  set sndbuf (Int64.max_int);
  print_endline ("Maximizing kernel receive buffer size.\n");
  set rcvbuf (Int64.max_int);
  get_options ()

let close () = 
  print_endline "Closing ZMQ socket.";
  close sock;
  print_endline "Closing ZMQ context.";
  term ctxt;
  exit 0

let _ = 
  ignore(change_options (get_options ()));
  ignore(maximize_options ());
  close ()
