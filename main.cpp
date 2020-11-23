int client_start(int argc, char *argv[]);

/* project entry */
int main(int argc, char *argv[])
{
	int ret = 0;

	ret = client_start(argc, argv);

	return ret;
}
