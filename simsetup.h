
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/
/*Set up the simulator stuff!!!
/**/
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/

/* Given an argc and an argv, process the arguments and set global */
 /* variables.  Some of these only make sense at startup.  Others */
 /* don't make sense at start up. */
void get_globals(int argc, char *argv[])

{
  /* Command line params. */
  while (argc > 1) {
    /* Two argument switches. */
    if (argc == 2) {
      fprintf(stderr, "\"%s\" argument needs a value.\n", argv[1]);
      exit(-1);
    }
    if (!strcmp(argv[1], "strat")) {
      if (!strcmp(argv[2], "RANDOM"))
	strat = RANDOM;
      else if (!strcmp(argv[2], "LEARN"))
	strat = LEARN;
      else if (!strcmp(argv[2], "BEST"))
	strat = BEST;
      else if (!strcmp(argv[2], "BESTATIC"))
	strat = BESTATIC;
      else if (!strcmp(argv[2], "PRIOR"))
	strat = PRIOR;
      else {
	fprintf(stderr, "strat \"%s\" not recognized.\n", argv[2]);
	exit(-1);
      }
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "learntype")) {
      if (!strcmp(argv[2], "QL"))
	learntype = QL;
      else if (!strcmp(argv[2], "VL"))
	learntype = VL;
      else {
	fprintf(stderr, "learntype \"%s\" not recognized.\n", argv[2]);
	exit(-1);
      }
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "interreport")) {
      interreport = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "quit_criteria")) {
      quit_criteria = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "queuelimit")) {
      queuelimit = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "maxpackets")) {
      maxpackets = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "graphname")) {
      graphname = argv[2];
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "eta")) {
      printf("Literally how could I get here more than once?\n" );
      eta = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "vprob")) {
      vprob = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "internode")) {
      internode = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "callmean")) {
      callmean = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "callstd")) {
      callstd = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "epsilon")) {
      epsilon = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "queuelevels")) {
      queuelevels = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "interqueue")) {
      interqueue = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "polfile")) {
      polfile = argv[2];
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "up")) {	/* no sense at startup. */
      int edge, n1, n2;
      edge = atoi(argv[2]);
      n1 = edge_from[edge];
      n2 = edge_to[edge];
      down[n1][n2] = 0;
      down[n2][n1] = 0;
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "down")) { /* no sense at startup. */
      int edge, n1, n2;
      edge = atoi(argv[2]);
      n1 = edge_from[edge];
      n2 = edge_to[edge];
      down[n1][n2] = 1;
      down[n2][n1] = 1;
      argc -= 2;
      argv += 2;
    }
    else {
      fprintf(stderr, "Switch not recognized: %s\n", argv[1]);
      exit(-1);
    }
  }
}

/* Setup graph parameters. */
void init_graph(char *filename)
{
  char line[300];	/* Line from the file. */
  FILE *graph;
  int link, node, node1, node2;
  int dummy;
  double x, y;

  graph = fopen(filename, "r");	/* Try to open it. */
  if (graph == NULL) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    exit(-1);
  }

  nnodes = 0;
  nedges = 0;
  while (fscanf(graph, "%[^\n]\n", line) != EOF) {
    if(!strncmp(line, "1000 ", 5)) {	/* Node declaration. */
      sscanf(line, "1000 %d %f %f %d", &node, &x, &y, &dummy);
      if (node != nnodes) {
	fprintf(stderr, "bad graph format: got node %d after %d, expected %d.\n",
		node, nnodes-1, nnodes);
	exit(-1);
      }
      /* Check for overflow. */
      if (nnodes == MAXNODES) {
	fprintf(stderr, "MAXNODES too small for this graph.\n");
	exit(-1);
      }
      /* Init new node. */
      xnode[nnodes] = x;
      ynode[nnodes] = y;
      nlinks[nnodes] = 0;
      nnodes++;
    }
    else if (!strncmp(line, "2000 ", 5)) {	/* Link declaration. */
      sscanf(line, "2000 %d %d %d ", &node1, &node2, &dummy);

      /* Check if broken. */
      if ((node1 >= nnodes) || (node2 >= nnodes) ||
	  (node1 < 0) || (node2 < 0)) {
	fprintf(stderr, "Link bad nodes (too big or too small):\n%s\n.",
		line);
	fprintf(stderr, "parsed: %d, %d\n", node1, node2);
	exit(-1);
      }

      /* Check for link overflow. */
      if (nlinks[node1] == MAXLINKS) {
	fprintf(stderr, "MAXLINKS too small for node: %d\n", node1);
	exit(-1);
      }
      links[node1][nlinks[node1]] = node2;
      nlinks[node1]++;

      /* Add bidirectional link. */
      if (nlinks[node2] == MAXLINKS) {
	       fprintf(stderr, "MAXLINKS too small for node: %d\n", node2);
	        exit(-1);
      }
      links[node2][nlinks[node2]] = node1;
      nlinks[node2]++;

      /* Add the edge.  Should check for nedges overflow. */
      edge_from[nedges] = node1;
      edge_to[nedges] = node2;
      nedges++;
    }

    /* Skip any other type of line. */
  }
  /* Done reading.  What did we get? */
  fprintf(stderr, "Read %d nodes\n", nnodes);
  fprintf(stderr, "   time  activ deliv avg xmit avg hop    oldest    mean    queue_  send_\n         pakts pakts   time     time     packet     age     full?   fail?\n");
}


/* Before calling do_sim. */
void do_sim_setup()
{
  int e;	/* Event to handle. */
  int i, j;

  /* Set up the "hardware". */
  for (i = 0; i < nnodes; i++) interqueuen[i] = interqueue;

  /* Links are "up" to start. */
  for (i = 0; i < nnodes; i++)
    for (j = 0; j < nnodes; j++)
      down[i][j] = 0;

  /* Initialize some structures. */
  freelist = Nil;			/* Create free list. */
  for (e = 0; e < MAXEVENTS; e++)
    free_event(e);

  /* Put in initial injection and report events. */
  queuetop = Nil;
  e = create_event(0.0, 0);
  events[e].source = REPORT;
  events[e].etime = 0.0;	/* Do this right away. */
  push_event(e, 0.0);
  e = create_event(0.0, 0);
  events[e].source = INJECT;
  events[e].etime = poisson(callmean);
  push_event(e, 0.0);

  /* Empty all node queues. */
  for (i = 0; i < nnodes; i++) {
    enqueued[i] = 0.0;
    nenqueued[i] = 0;
  }
}
