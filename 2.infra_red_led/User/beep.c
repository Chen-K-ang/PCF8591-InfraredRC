void beep(void);
{
	Beep=0;
Delays(50);
	Beep=1;
}
void    Delays(unsigned int xms)
{
    unsigned int i, j;
    for (i = xms; i > 0; i--)
            for (j = 110; j > 0; j--);

}