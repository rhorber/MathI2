/*
 * gmv.c -- the goto virtual machine
 *
 * (c) 2012-2024 Prof Dr Andreas Mueller
 */
#define _DEFAULT_SOURCE
#include <includes.h>
#include <gvm.h> 
#include <utils.h>

/* some variables and functions needed by the bison-generated parser */
extern int	yydebug;

void    yyerror(char *errmsg) {
	fprintf(stderr, "error: %s\n", errmsg);
}

int     yywrap() {
	return 1;
}

extern int	yyparse();
extern int	yylex();

/* initialize the virtual machine structure */
gvm_t	gvm = {
	.nnodes = 0,
	.nodes = NULL,
	.nvars = 0,
	.vars = NULL,
	.sleep = 0,
};

/* usage function */
static void	usage(const char *progname) {
	printf("usage: %s [ -dnth? ] [ -0 var0value ] [ -s interval ] file.goto\n\n", progname);
	printf("run a GOTO program in file file.goto\n\n");
	printf("options:\n");
	printf("  -0,--vzero=<var0value>  initialize variable 0 with value var0value instead\n");
	printf("                          of the default 0. This is the only parameter\n");
	printf("                          transfer method to the virtual machine.\n");
	printf("  -d,--debug              debug mode, show many (difficult to understand)\n");
	printf("                          details about the parse and execute process\n");
	printf("  -h,-?,--help            show this help message and exit\n");
	printf("  -s,--sleep=<interval>   pause for interval microseconds between two commands\n");
	printf("                          of the GOTO program\n");
	printf("  -t,--trace              trace mode, shows result of each executed step\n");
}

static struct option	options[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "norun",	no_argument,		NULL,		'n' },
{ "trace",	no_argument,		NULL,		't' },
{ "sleep",	required_argument,	NULL,		's' },
{ "vzero",	required_argument,	NULL,		'v' },
{ NULL,		0,			NULL,		 0  }
};


/* main function */
int	main(int argc, char *argv[]) {
        yydebug = 0;
        int     c;
	int	longindex;
	int	norun = 0;
        while (EOF != (c = getopt_long(argc, argv, "dnt0:s:h?",
			options, &longindex)))
                switch (c) {
                case 'd':       yydebug = 1;
                                break;
		case 'n':	norun = 1;
				break;
		case 't':	gvm_trace = 1;
				break;
		case '0':	gvm_initvar(&gvm, optarg);
				break;
		case 's':	gvm.sleep = atoi(optarg);
				break;
		case '?':
		case 'h':	usage(argv[0]);
				exit(EXIT_SUCCESS);
                }

	/* remaining parameter should be the file to read */
	if (optind >= argc) {
		fprintf(stderr, "no program argument\n");
		exit(EXIT_FAILURE);
	}
	char	*filename = argv[optind];
	if (yydebug) {
		printf("opening file %s\n", filename);
	}
	gvm_file = open(filename, O_RDONLY, 0);
	if (gvm_file < 0) {
		fprintf(stderr, "cannot open file %s: %s\n", filename,
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* parse the program */
        yyparse();

	/* display the program */
	if (yydebug) {
		gvm_display(&gvm);
	}

	/* execute the program */
	if (norun) {
		exit(EXIT_SUCCESS);
	}
	gvm_run(&gvm);

	exit(EXIT_SUCCESS);
}
