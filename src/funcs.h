#ifndef CONFIG_H
#define CONFIG_H
#include "inkview.h"

/* function description */
typedef struct btn_fun_t{
	const char name[16];    /* function name (button label) */
    const char str[6][16];  /* function expression (e.g. sin() can be declared as 3 words: { "sin", "(", ")" } */
	unsigned pos;           /* cursor offset from the end of the functon expression */
	bool is_regular_button; /* is this just regular button (Button class), or CalcButton */
} button_func;              /* (e.g. to set cursor in "()*180/pi" between parentheses, pos must be equal to 5) */ 

/* some UI parameters */ 
#define CFG_FONT_NAME DEFAULTFONTM
#define CFG_TEXTBOX_FONT_SIZE 16
/* this one can be decreased by application if screen isn't large enought */
#define CFG_BUTTON_FONT_SIZE 20

/* helpers macro */
#define F_FULL(name, w1, w2, w3, w4, w5, w6,  pos) {name, {w1,w2,w3,w4,w5,w6},pos, false}
#define F_SIMPLE(name) {name,{name,"","","","",""}, 0, false}
#define F_1W(name, w1) {name, {w1,"","","","",""}, 0, false}
#define F_2W(name, w1, w2) {name, {w1, w2,"","",""}, 0, false}
#define F_3W(name, w1, w2, w3) {name, {w1,w2,w3,"","",""}, 1, false}
#define F_SEP {"",{"","","","","",""},0, false}
#define F_BR(name,func) {name,{func,"(",")","","",""}, 1, false}
#define F_BUTTON(label) {label,{"","","","","",""}, 0, true}


#define CFG_GRID_COLS 10
#define CFG_GRID_ROWS 8

/* functions */
button_func functions[CFG_GRID_ROWS][CFG_GRID_COLS] = {
	/* row 1 */
	{ 
		F_SIMPLE("7"),
		F_SIMPLE("8"),
		F_SIMPLE("9"),
		F_SIMPLE("/"),
		F_SIMPLE("("),
		F_SEP,
		F_FULL("E","*","10","^","(",")","",1),
		F_BR("exp","exp"),
		F_SIMPLE("pi"),
		F_BUTTON("expr")
	},
	/* row 2 */
	{
		F_SIMPLE("4"),
		F_SIMPLE("5"),
		F_SIMPLE("6"),
		F_SIMPLE("*"),
		F_SIMPLE(")"),
		F_SEP,
		F_BR("rad","rad"),
		F_BR("deg","deg"),
		F_BR("abs","abs"),
		F_SIMPLE("ans")
	},
	/* row 3 */
	{
		F_SIMPLE("1"),
		F_SIMPLE("2"),
		F_SIMPLE("3"),
		F_SIMPLE("-"),
		F_SIMPLE("^"),
		F_SEP,
		F_SIMPLE(","),
		F_SEP,
		F_SEP,
		F_SEP
	},
	/* row 4 */
	{
		F_SIMPLE("."),
		F_SIMPLE("0"),
		F_SIMPLE(":="),
		F_SIMPLE("+"),
		F_BR("sqrt","sqrt"),
		F_SEP,
		F_SIMPLE("a"),
		F_SIMPLE("b"),
		F_SIMPLE("c"),
		F_SIMPLE("d")
	},
	/* row 5 */
	{
		F_SEP,F_SEP,F_SEP,F_SEP,F_SEP,F_SEP,F_SEP,F_SEP,F_SEP,F_SEP
	},
	/* row 6 */
	{
		F_BR("sin","sin"),
		F_BR("cos","cos"),
		F_BR("tan","tan"),
		F_BR("cot","cot"),
		F_BR("asin","asin"),
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP
	},
	/* row 7 */
	{
		F_BR("acos","acos"),
		F_BR("atan","atan"),
		F_BR("acot","acot"),
		F_BR("ln","ln"),
		F_BR("log10","log10"),
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP
	},
	/* row 8 */
	{
		F_BR("log2","log2"),
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP,
		F_SEP
	}
};
#endif
