--- ./macof.c.orig	2001-03-15 09:33:04.000000000 +0100
+++ ./macof.c	2014-07-22 13:20:14.000000000 +0200
@@ -48,8 +48,8 @@
 static void
 gen_mac(u_char *mac)
 {
-	*((in_addr_t *)mac) = libnet_get_prand(PRu32);
-	*((u_short *)(mac + 4)) = libnet_get_prand(PRu16);
+	*((in_addr_t *)mac) = libnet_get_prand(LIBNET_PRu32);
+	*((u_short *)(mac + 4)) = libnet_get_prand(LIBNET_PRu16);
 }
 
 int
@@ -59,22 +59,23 @@
 	extern int optind;
 	int c, i;
 	struct libnet_link_int *llif;
-	char ebuf[PCAP_ERRBUF_SIZE];
+	char pcap_ebuf[PCAP_ERRBUF_SIZE];
+	char libnet_ebuf[LIBNET_ERRBUF_SIZE];
 	u_char sha[ETHER_ADDR_LEN], tha[ETHER_ADDR_LEN];
 	in_addr_t src, dst;
 	u_short sport, dport;
 	u_int32_t seq;
-	u_char pkt[ETH_H + IP_H + TCP_H];
+	libnet_t *l;
 	
 	while ((c = getopt(argc, argv, "vs:d:e:x:y:i:n:h?V")) != -1) {
 		switch (c) {
 		case 'v':
 			break;
 		case 's':
-			Src = libnet_name_resolve(optarg, 0);
+			Src = libnet_name2addr4(l, optarg, 0);
 			break;
 		case 'd':
-			Dst = libnet_name_resolve(optarg, 0);
+			Dst = libnet_name2addr4(l, optarg, 0);
 			break;
 		case 'e':
 			Tha = (u_char *)ether_aton(optarg);
@@ -101,13 +102,13 @@
 	if (argc != 0)
 		usage();
 	
-	if (!Intf && (Intf = pcap_lookupdev(ebuf)) == NULL)
-		errx(1, "%s", ebuf);
+	if (!Intf && (Intf = pcap_lookupdev(pcap_ebuf)) == NULL)
+		errx(1, "%s", pcap_ebuf);
 	
-	if ((llif = libnet_open_link_interface(Intf, ebuf)) == 0)
-		errx(1, "%s", ebuf);
+	if ((l = libnet_init(LIBNET_LINK, Intf, libnet_ebuf)) == NULL)
+		errx(1, "%s", libnet_ebuf);
 	
-	libnet_seed_prand();
+	libnet_seed_prand(l);
 	
 	for (i = 0; i != Repeat; i++) {
 		
@@ -117,39 +118,39 @@
 		else memcpy(tha, Tha, sizeof(tha));
 		
 		if (Src != 0) src = Src;
-		else src = libnet_get_prand(PRu32);
+		else src = libnet_get_prand(LIBNET_PRu32);
 		
 		if (Dst != 0) dst = Dst;
-		else dst = libnet_get_prand(PRu32);
+		else dst = libnet_get_prand(LIBNET_PRu32);
 		
 		if (Sport != 0) sport = Sport;
-		else sport = libnet_get_prand(PRu16);
+		else sport = libnet_get_prand(LIBNET_PRu16);
 		
 		if (Dport != 0) dport = Dport;
-		else dport = libnet_get_prand(PRu16);
+		else dport = libnet_get_prand(LIBNET_PRu16);
 
-		seq = libnet_get_prand(PRu32);
-		
-		libnet_build_ethernet(tha, sha, ETHERTYPE_IP, NULL, 0, pkt);
-		
-		libnet_build_ip(TCP_H, 0, libnet_get_prand(PRu16), 0, 64,
-				IPPROTO_TCP, src, dst, NULL, 0, pkt + ETH_H);
+		seq = libnet_get_prand(LIBNET_PRu32);
 		
 		libnet_build_tcp(sport, dport, seq, 0, TH_SYN, 512,
-				 0, NULL, 0, pkt + ETH_H + IP_H);
+				 0, 0, LIBNET_TCP_H, NULL, 0, l, 0);
 		
-		libnet_do_checksum(pkt + ETH_H, IPPROTO_IP, IP_H);
-		libnet_do_checksum(pkt + ETH_H, IPPROTO_TCP, TCP_H);
+		libnet_build_ipv4(LIBNET_TCP_H, 0,
+				  libnet_get_prand(LIBNET_PRu16), 0, 64,
+				  IPPROTO_TCP, 0, src, dst, NULL, 0, l, 0);
 		
-		if (libnet_write_link_layer(llif, Intf, pkt, sizeof(pkt)) < 0)
+		libnet_build_ethernet(tha, sha, ETHERTYPE_IP, NULL, 0, l, 0);
+		
+		if (libnet_write(l) < 0)
 			errx(1, "write");
 
+		libnet_clear_packet(l);
+
 		fprintf(stderr, "%s ",
 			ether_ntoa((struct ether_addr *)sha));
 		fprintf(stderr, "%s %s.%d > %s.%d: S %u:%u(0) win 512\n",
 			ether_ntoa((struct ether_addr *)tha),
-			libnet_host_lookup(Src, 0), sport,
-			libnet_host_lookup(Dst, 0), dport, seq, seq);
+			libnet_addr2name4(Src, 0), sport,
+			libnet_addr2name4(Dst, 0), dport, seq, seq);
 	}
 	exit(0);
 }
