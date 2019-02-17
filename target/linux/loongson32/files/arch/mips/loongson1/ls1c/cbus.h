#ifndef _GPIO_CBUS_H_
#define _GPIO_CBUS_H_
/* set gpio func CBUS */

//void cbus_dump(void);
//void gpio_func(uint8_t func ,uint8_t gpio_no);
//void gpio_func_dis(uint8_t func,uint8_t gpio_no);
#define CBUS_MUX(func, r) \
                ((void __iomem *)0xbfd011c0 + ((func -1) * 0x10) + (4*r))
#define CBUSS(func, r) \
                0x11c0 + ((func -1) * 0x10) + (4*r)
void cbus_dump(void) {
#ifdef CONFIG_LS1X_CBUS_DUMP
	int16_t i;
	char buff[500];
	printk(KERN_ERR "gpio  ==SHUT_CTRL=[%08x]==uart0fu=[%08x]==uart8fu=[%08x]=========\n",__raw_readl(LS1X_MUX_CTRL0),__raw_readl((void __iomem *)0xbfd00420),__raw_readl((void __iomem *)0xbfe4c904));
	printk(KERN_ERR "gpio  31-----0 63----32 95----64 127---96\n");

	for(i=0;i<5;i++) {
		printk(KERN_ERR "fun%d: %08x %08x %08x %08x\n",i+1,
__raw_readl((void __iomem *)0xbfd011c0+i*0x10),
__raw_readl((void __iomem *)0xbfd011c0+i*0x10+4),
__raw_readl((void __iomem *)0xbfd011c0+i*0x10+8),
__raw_readl((void __iomem *)0xbfd011c0+i*0x10+0xc));
		//printk(KERN_ERR buff);
	}
for(i=0;i<128;i++) {
sprintf(buff,"gpio%d:fun",i);
if(__raw_readl((void __iomem *)0xbfd011c0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s1\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd011d0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s2\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd011e0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s3\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd011f0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s4\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd01200+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s5\n",buff);
continue;
}
}

#endif// CONFIG_LS1X_CBUS_DUMP
}

void gpio_func(uint8_t func,uint8_t gpio_no) {
	uint8_t i,regno,regbit;
	uint32_t data=0;
	regno=(uint8_t) gpio_no/32;
	regbit=gpio_no % 32;
	//cbus_dump();
	data=__raw_readl(CBUS_MUX(func,regno));
	for(i=1;i<6;i++){
		if(i<func)
			__raw_writel(__raw_readl(CBUS_MUX(i,regno)) & ~(1<<regbit),CBUS_MUX(i,regno));
		else {
			__raw_writel(__raw_readl(CBUS_MUX(i,regno)) | (1<<regbit),CBUS_MUX(i,regno));
			break;
		}
	}
	printk(KERN_ERR "set gpio%d,fun%d,addr=[0xbfd0%x] %08x | %08x = %08x\n", gpio_no,func,CBUSS(func,regno),data,1<<regbit,__raw_readl(CBUS_MUX(func,regno)));
	//cbus_dump();
}

void gpio_func_dis(uint8_t func,uint8_t gpio_no) {
	uint8_t regno,regbit;
	regno=(uint8_t) gpio_no/32;
	regbit=gpio_no % 32;
	__raw_writel(__raw_readl(CBUS_MUX(func,regno)) & ~(1<<regbit),CBUS_MUX(func,regno));
	printk(KERN_ERR "disable gpio%d,fun%d\n", gpio_no,func);

}

#endif
