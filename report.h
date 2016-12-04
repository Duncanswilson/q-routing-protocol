
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/
/* Report Stuff
/**/
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
int nodes_alive;
double oldest_node;
double total_age;
#define MEAN(x) (DOUBLE(x)/DOUBLE(nodes_alive))

void report0(int h1, double  time)
{
  double age;

  if (h1 == Nil) return;
  if (events[h1].source >= 0) {
    nodes_alive++;
    age = time - events[h1].birth;
    if (age > oldest_node) oldest_node = age;
    total_age += age;
  }
  report0(events[h1].left, time);
  report0(events[h1].right, time);
}

void dump_dest(int node)
{
  int j, l;

  printf("{");
  for (j = 0; j < nnodes; j++) {
    l = choose_link(j, node);
    printf("%d ", links[j][l]);
  }
  printf("}\n");
  fflush(stdout);
}

void interactive_report(double time, int e)
{
  int argc;
  char *argv[100];
  char commands[1000], word[1000];
  double next_report = time+1000.0;
  int i, badcmd;
  int n1, n2, edge;
  char *ptr;

  if (time > 0.0) {
    printf("{%f %f %f} ", time, DOUBLE(total_routing_time)/DOUBLE(routed_packets),
	   DOUBLE(total_hops)/DOUBLE(routed_packets));
    printf("{");
    for (i = 0; i < nedges; i++)
      printf("%d ", link_activity[edge_from[i]][edge_to[i]]
	     +link_activity[edge_to[i]][edge_from[i]]);
    printf("}\n");
    fflush(stdout);

  }
  routed_packets = 0;	/* Reset. */
  total_routing_time = 0;
  total_hops = 0;
  for (i = 0; i < nedges; i++)
    link_activity[edge_from[i]][edge_to[i]] =
      link_activity[edge_to[i]][edge_from[i]] = 0;

  badcmd = 1;
  /* loop until a continuation command appears. */
  while (badcmd) {
    gets(commands);
    /* Parse into argc, argv (trailing blank counts as an argument). */
    argc = 1;	/* argv[0] doesn't count. */
    ptr = commands;
    while (*ptr) {
      while (*ptr && *ptr == ' ') ptr++;	/* Skip leading spaces. */
      argv[argc] = ptr;			/* That's the start. */
      while (*ptr && *ptr != ' ') ptr++;	/* Skip non-spaces. */
      if (*ptr) *(ptr++) = '\0';		/* Mark the end. */
      argc++;
    }
    /* Catch policy request.  (ignores other commands on line). */
    if ((argc >= 3) && (!strcmp("pol", argv[1]))) {
      dump_dest(atoi(argv[2]));
    }
    else badcmd = 0;
  }

  get_globals(argc, argv);

  events[e].etime += interreport;
  push_event(e, time);
}
