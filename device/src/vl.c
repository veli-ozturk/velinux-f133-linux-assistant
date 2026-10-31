/* vl: VeLinux KOLE cok-cagrili araclar (tek statik ikili, glibc bir kez). argv[0] adina gore:
 *   acilis (acilis.c), fbal (fbal.c), mcu (mcu.c), pinizle (pinizle.c). "vl <arac> ..." de olur. */
#include <stdio.h>
#include <string.h>
int acilis_main(int, char **);
int fbal_main(void);
int mcu_main(int, char **);
int pinizle_main(int, char **);
int main(int argc, char **argv)
{
	const char *a = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
	if (!strcmp(a, "vl") && argc > 1) {
		argv++;
		argc--;
		a = argv[0];
	}
	if (!strcmp(a, "acilis"))
		return acilis_main(argc, argv);
	if (!strcmp(a, "fbal"))
		return fbal_main();
	if (!strcmp(a, "mcu"))
		return mcu_main(argc, argv);
	if (!strcmp(a, "pinizle"))
		return pinizle_main(argc, argv);
	fprintf(stderr, "vl: acilis | fbal | mcu | pinizle\n");
	return 2;
}
