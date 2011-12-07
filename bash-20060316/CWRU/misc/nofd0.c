main(c, v, e)
int	c;
char	**v, **e;
{
	close(0);
	execv(v[1], v+1);
}

	
