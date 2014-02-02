#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <cmath>
#include <utility>
#include <functional>
#include <iostream>
#include <clocale>
#include <deque>
#include "inkview.h"
#include "fparser.hh"
#include "funcs.h"

#define uint unsigned int
#define ushort unsigned short

#define __DEBUG_ENABLED

#ifdef __DEBUG_ENABLED
	#define __DBG(x) std::cout << "__DBG in " << __FUNCTION__ << " at line " <<  __LINE__ << ": " << x << std::endl;
#else
	#define __DBG(x) ;
#endif

const unsigned EVT_BASE = 2000;
//TODO: split this file
const unsigned EVT_BUTTON_ACTIVATE = EVT_BASE + 1; //button click
const unsigned EVT_CALC_BUTTON_ACTIVATE = EVT_BASE + 2; //click on a calcbutton
const unsigned EVT_TEXTBOX_POSITION = EVT_BASE + 3; //sent when changing cursor position in textbox
const unsigned EVT_MENU_SELECT = EVT_BASE + 4; //sent when menu item is activated
const unsigned EVT_LIST_ACTION = EVT_BASE + 5; //FullscreenList action (e.g. close, open, menu)
const unsigned EVT_KEYBOARD = EVT_BASE + 6; //sent when the keyboard is closed

using std::string;
using std::vector;
using std::deque;

/* ****************** forward declarations ********************************** */
int global_event_handler(int, int, int);

/* ***************** Simple abstract widget class *************************** */
class Widget {
public:
	Widget() {
		this->m_id = m_last_id++;
		this->m_label = "";
		this->m_x = 0;
		this->m_y = 0;
		this->m_w = 0;
		this->m_h = 0;
		this->m_font = NULL;
		this->m_visible = true;

		m_all_widgets[this->getID()] = this;
	}

	Widget(const string& label, unsigned x, unsigned y, unsigned w, unsigned h) {
		this->m_id = m_last_id++;
		this->m_label = label;
		this->m_x = x;
		this->m_y = y;
		this->m_w = w;
		this->m_h = h;
		this->m_font = NULL;
		this->m_visible = true;

		m_all_widgets[this->getID()] = this;
	}

	virtual ~Widget() {
		for(std::map<unsigned, Widget*>::iterator it = m_all_widgets.begin(); it != m_all_widgets.end(); it++)
			if((*it).second == this) {
				m_all_widgets.erase(it);
				break;
			}

		if(Widget::m_focus == this)
			m_focus = NULL;
	}

	virtual void draw() const = 0;

	virtual void update() {
		PartialUpdateBW(m_x, m_y, m_w, m_h);
	}

	virtual void asyncUpdate() {
		DynamicUpdateBW(m_x, m_y, m_w, m_h);
	}

	unsigned getID() const {
		return this->m_id;
	}

	const string& getLabel() const {
		return this->m_label;
	}

	unsigned getX() const {
		return m_x;
	}

	unsigned getY() const {
		return m_y;
	}

	unsigned getWidth() const {
		return m_w;
	}

	unsigned getHeight() const {
		return m_h;
	}

	virtual unsigned getMinWidth() const = 0;
	virtual unsigned getMinHeight() const = 0;

	void setSize(unsigned w, unsigned h) {
		this->m_w = w;
		this->m_h = h;
	}

	void setPos(unsigned x, unsigned y) {
		this->m_x = x;
		this->m_y = y;
	}

	void setVisibility(bool is_visible) {
		m_visible = is_visible;
	}
	
	bool getVisibility() const {
		return m_visible;
	}
	
	static void hideAll() {
		for(std::map<unsigned, Widget*>::iterator it = m_all_widgets.begin();
			it != m_all_widgets.end(); it++)
		{
			(*it).second->setVisibility(false);
		}
	}
	
	static void showAll() {
		for(std::map<unsigned, Widget*>::iterator it = m_all_widgets.begin();
			it != m_all_widgets.end(); it++)
		{
			(*it).second->setVisibility(true);
		}
	}
	
	static void processEvent(unsigned id, unsigned param1, unsigned param2) {
		if(id == EVT_KEYRELEASE) {
			if(m_focus) {
				m_focus->onKeyUp(param1);
			}
		}
		else if(id == EVT_POINTERUP || id == EVT_POINTERLONG) {
			for(std::map<unsigned, Widget*>::iterator it = m_all_widgets.begin();
			    it != m_all_widgets.end();
			    it++) {
				Widget* w = (*it).second;
				if(param1 >= w->m_x && param2 >= w->m_y &&
				   param1 < (w->m_x + w->m_w) &&
				   param2 < (w->m_y + w->m_h) && w->getVisibility()) {
					if(id != EVT_POINTERLONG)
						w->onTouchDown(param1, param2);
					else
						w->onTouchLong(param1, param2);
					break;
				}
			}
		}
	}

	static void drawAll() {
		for(std::map<unsigned, Widget*>::iterator it = m_all_widgets.begin();
		    it != m_all_widgets.end();
		    it++)
		{
			if((*it).second->getVisibility())
				(*it).second->draw();
		}
	}

	static void setFocus(Widget* focus, bool update_previous = false) {
		if(!focus->getVisibility())
			return;
		
		Widget* tmp = m_focus;
		m_focus = focus;

		if(tmp && update_previous) {
			tmp->draw();
			tmp->update();
		}
	}

	void setFont(ifont* widget_font) {
		m_font = widget_font;
	}

	static Widget* getFocus() {
		return m_focus;
	}

	static void setGlobalFont(ifont* widgets_font) {
		m_global_font = widgets_font;
	}

	static const ifont* getGlobalFont() {
		return m_global_font;
	}

	const ifont* getFont() const {
		return ((m_font) ? m_font : m_global_font);
	}

	static Widget* getByID(unsigned id) {
		return (m_all_widgets.find(id) != m_all_widgets.end()) ? m_all_widgets[id] : NULL;
	}
	
	static Widget* findByName(const string& name) {
		for(std::map<uint,Widget*>::iterator it = m_all_widgets.begin();
			it != m_all_widgets.end();
			it++)
		{
			if((*it).second->getLabel() == name)
				return (*it).second;
		}
		
		return NULL;
	}

protected:
	virtual void onTouchDown(unsigned, unsigned) = 0;
	virtual void onTouchLong(unsigned, unsigned) {}
	virtual void onKeyUp(unsigned key) = 0;

	string m_label;

	unsigned m_x;
	unsigned m_y;
	unsigned m_w;
	unsigned m_h;
	bool m_visible;
	
	static Widget* m_focus;

	ifont* m_font;
	static ifont* m_global_font;
private:
	unsigned m_id;
	static unsigned m_last_id;

	static std::map<unsigned, Widget*> m_all_widgets;
};

ifont* Widget::m_global_font = NULL;
unsigned Widget::m_last_id = 0;
std::map<unsigned, Widget*> Widget::m_all_widgets;
Widget* Widget::m_focus = NULL;

/* **************** Simple grid layout class for widgets ******************** */

class GridLayout {
public:
	GridLayout(unsigned x, unsigned y, unsigned rows, unsigned cols,
	           unsigned width, unsigned height, unsigned sp) :
		m_widgets(rows), m_x(x), m_y(y), m_width(width), m_height(height), m_spacing(sp) {
		for(size_t i = 0; i < rows; i++) {
			m_widgets[i].resize(cols);

			for(size_t j = 0; j < cols; j++)
				m_widgets[i][j] = NULL;
		}
	}

	bool append(const Widget* widget) {
		for(size_t i = 0; i < m_widgets.size(); i++) {
			for(size_t j = 0; j < m_widgets[0].size(); j++)
				if(m_widgets[i][j] == NULL) {
					m_widgets[i][j] = const_cast<Widget*>(widget);

					return true;
				}
		}
		return false;
	}

	void insert(const Widget* widget, unsigned row, unsigned col) {
		m_widgets[row][col] = const_cast<Widget*>(widget);
	}

	const vector<Widget*>& operator[](unsigned row) const {
		return m_widgets[row];
	}

	unsigned getWidth() const {
		return m_width;
	}

	unsigned getHeight() const {
		return m_height;
	}

	void update() {
		unsigned x_paddings = (m_widgets[0].size()) ? m_widgets[0].size() - 1 : 0;
		unsigned y_paddings = (m_widgets.size()) ? m_widgets.size() - 1 : 0;

		unsigned emptyCols = getEmptyCols();
		unsigned emptyRows = getEmptyRows();

		unsigned opt_width = (m_width - m_spacing * x_paddings) / (m_widgets[0].size() - emptyRows);
		unsigned opt_height = (m_height - m_spacing * y_paddings) / (m_widgets.size() - emptyCols);

		unsigned row_height = 0;
		unsigned col_width = 0;

		unsigned x, y;

		//FIXME: assuming that global font is used for all widgets
		SetFont(const_cast<ifont*>(Widget::getGlobalFont()), BLACK);

		y = 0;
		for(unsigned row = 0; row < m_widgets.size(); row++) {
			x = 0;
			row_height = getMinRowHeight(row);
			if(row_height && row_height < opt_height)
				row_height = opt_height;

			for(unsigned col = 0; col < m_widgets[0].size(); col++) {
				col_width = getMinColWidth(col);
				if(col_width && col_width < opt_width)
					col_width = opt_width;

				if(m_widgets[row][col]) {
					m_widgets[row][col]->setPos(
					    m_x + x + (col ? m_spacing : 0),
					    m_y + y + (row ? m_spacing : 0)
					);

					m_widgets[row][col]->setSize(col_width, row_height);
				}

				x += (col ? m_spacing : 0) + col_width;
			}

			y += (row ? m_spacing : 0) + row_height;
		}
	}

	void __debug_draw() {
		DrawRect(m_x, m_y, m_width, m_height, LGRAY);
	}

private:
	static bool cmpMinHeight(const Widget* x1, const Widget* x2) {
		unsigned h1 = ((x1) ? (x1->getMinHeight()) : 0);
		unsigned h2 = ((x2) ? (x2->getMinHeight()) : 0);
		return h1 < h2;
	}
	static struct cmpWidth_t {
		unsigned col;
		bool operator()(const vector<Widget*>& x1, const vector<Widget*>& x2) {
			unsigned w1 = ((x1[col]) ? (x1[col]->getMinWidth()) : 0);
			unsigned w2 = ((x2[col]) ? (x2[col]->getMinWidth()) : 0);
			return w1 < w2;
		}
	} cmpMinWidth;

	unsigned getMinRowHeight(unsigned row) {
		Widget* w = (*std::max_element(m_widgets[row].begin(), m_widgets[row].end(), cmpMinHeight));
		return ((w) ? w->getMinHeight() : 0);
	}

	unsigned getMinColWidth(unsigned col) {
		cmpMinWidth.col = col;
		Widget* w = (*std::max_element(m_widgets.begin(), m_widgets.end(), cmpMinWidth)) [col];
		return ((w) ? w->getMinWidth() : 0);
	}

	unsigned getEmptyRows() {
		unsigned rows = 0;
		for(unsigned row = 0; row < m_widgets.size(); row++) {
			if((unsigned) std::count(m_widgets[row].begin(), m_widgets[row].end(), (Widget*) NULL) ==
			   m_widgets[row].size())
				rows++;
		}
		return rows;
	}

	unsigned getEmptyCols() {
		unsigned cols = 0;
		for(unsigned col = 0; col < m_widgets[0].size(); col++) {
			bool empty_col = true;
			for(unsigned row = 0; row < m_widgets.size(); row++) {
				if(m_widgets[row][col] != NULL) {
					empty_col = false;
					break;
				}
			}
			if(empty_col)
				cols++;
		}
		return cols;
	}


	vector<vector<Widget*> > m_widgets;
	unsigned m_x;
	unsigned m_y;
	unsigned m_width;
	unsigned m_height;
	unsigned m_spacing;
};

GridLayout::cmpWidth_t GridLayout::cmpMinWidth = { 0 };

/* ************************ Button widget *********************************** */

class Button : public Widget {
public:
	Button(const string& label) :
		Widget(label, 0, 0, 0, 0) {
		c_activate_event = EVT_BUTTON_ACTIVATE;
	}

	Button(const string& label, unsigned x, unsigned y, unsigned w, unsigned h) :
		Widget(label, x, y, w, h) {
		c_activate_event = EVT_BUTTON_ACTIVATE;
	}

	void draw() const {
		if(m_focus == this)
			SetFont(const_cast<ifont*>(getFont()), WHITE);
		else
			SetFont(const_cast<ifont*>(getFont()), BLACK);

		if(m_focus == this)
			FillArea(m_x, m_y, m_w, m_h, BLACK);
		else {
			FillArea(m_x, m_y, m_w, m_h, WHITE);
			DrawRect(m_x, m_y, m_w, m_h, BLACK);
		}

		DrawTextRect(
		    m_x + m_padding, m_y + m_padding,
		    m_w - 2 * m_padding, m_h - 2 * m_padding,
		    m_label.c_str(), ALIGN_CENTER | VALIGN_MIDDLE
		);
	}

	unsigned getMinWidth() const {
		SetFont(const_cast<ifont*>(getFont()), BLACK);
		return StringWidth(m_label.c_str()) + 2 * m_padding;
	}

	unsigned getMinHeight() const {
		SetFont(const_cast<ifont*>(getFont()), BLACK);
		return TextRectHeight(40, "#", ALIGN_LEFT) + 2 * m_padding;
	}
protected:
	unsigned c_activate_event;
private:
	void onTouchDown(unsigned, unsigned) {
		if(getFocus() != this) {
			setFocus(this, true);
			InvertAreaBW(m_x, m_y, m_w, m_h);
			update();
		}

		SendEvent(&global_event_handler, c_activate_event, this->getID(), 0);
	}

	void onTouchLong(unsigned, unsigned) {
		if(getFocus() != this) {
			setFocus(this, true);
			InvertAreaBW(m_x, m_y, m_w, m_h);
			update();
		}

		SendEvent(&global_event_handler, c_activate_event, this->getID(), 1);
	}

	void onKeyUp(unsigned key) {
		if(key == KEY_OK) {
			__DBG("sending event");
			SendEvent(&global_event_handler, c_activate_event, this->getID(), 0);
			__DBG("event sent");
		}
	}

	static const ushort m_padding = 4;
};

/* ********************** Calculator button class *************************** */
class CalcButton : public Button {
public:
	CalcButton(const button_func* func) : Button(func->name) {
		m_func = const_cast<button_func*>(func);
		c_activate_event = EVT_CALC_BUTTON_ACTIVATE;
	}

	const button_func* getFunc() const {
		return m_func;
	}

private:
	button_func* m_func;
};

/* ****************** Text box widget *************************************** */

class TextBox : public Widget {
public:
	TextBox() : Widget() {
		m_drawCursor = false;
		m_pos = 0;
	}

	void insert(const string& str, unsigned pos) {
		m_words.insert(m_words.begin() + pos, str);
	}

	void append(const string& str) {
		m_words.push_back(str);
	}

	void clear() {
		m_words.clear();
		m_pos = 0;
		draw();
	}

	vector<string>& words() {
		return m_words;
	}

	void setDrawCursor(bool do_draw) {
		m_drawCursor = do_draw;
	}

	/* assuming monospace font is used */
	void draw() const {
		SetFont(const_cast<ifont*>(getFont()), BLACK);
		FillArea(m_x, m_y, m_w, m_h, WHITE);
		if(Widget::getFocus() == this)
			DrawSelection(m_x, m_y, m_w, m_h, BLACK);
		else
			DrawRect(m_x, m_y, m_w, m_h, BLACK);
		//DrawTextRect(e->x+4, e->y+4, e->width-4, e->height-4, e->text.c_str(), ALIGN_LEFT);

		string tmp;
		unsigned i = 0, yi = 0;
		unsigned dy = TextRectHeight(20, "#", 0);
		unsigned pos_x = m_x + 4, pos_y = m_y + 4;
		//TODO: break line only on operators, if possible + tune perf
		while(i < m_words.size() && yi < (m_h - 8) / dy) {
			while(i < m_words.size() && CharWidth('#') * (tmp.size() + m_words[i].size()) < m_w - 8) {
				if(i == m_pos) {
					pos_x = m_x + 4 + tmp.size() * CharWidth('#');
					pos_y = m_y + 4 + yi * dy;
				}

				if(i == m_words.size() - 1 && m_pos > i) {
					pos_x = m_x + 4 + (tmp.size() + m_words[i].size()) * CharWidth('#');
					pos_y = m_y + 4 + yi * dy;
				}

				tmp.append(m_words[i++]);
			}

			DrawString(m_x + 4, m_y + 4 + yi * dy, tmp.c_str());

			yi++;
			tmp.clear();
		}

		if(m_drawCursor)
			DrawLine(pos_x, pos_y, pos_x, pos_y + dy, BLACK);
	}

	unsigned getMinWidth() const {
		unsigned textlen = 0;
		for(size_t i = 0; i < m_words.size(); i++)
			textlen += m_words[i].size();
		return CharWidth('#') * sqrt(textlen) + 8 + 10;
	}
	unsigned getMinHeight() const {
		return getMinWidth();
	}

	void setTextPos(unsigned pos) {
		m_pos = pos;
	}

	unsigned getTextPos() const {
		return m_pos;
	}

	string getString() const {
		string tmp;
		for(uint i = 0; i < m_words.size(); i++)
			tmp.append(m_words[i]);
		return tmp;
	}

private:
	void onTouchDown(unsigned x, unsigned y) {
		Widget::setFocus(this, true);
		//TODO: improve performance (cache word coords or smth)
		unsigned width = 0;
		unsigned pos = 0;
		unsigned i = 0, yi = 0;
		unsigned dy = TextRectHeight(20, "#", 0);
		while(i < m_words.size() && yi < (m_h - 8) / dy) {
			while(i < m_words.size() && CharWidth('#') * (width + m_words.size()) < m_w - 8) {
				unsigned word_x = m_x + 4 + width * CharWidth('#');
				unsigned word_y = m_y + 4 + yi * dy;

				if(x >= word_x && x <= (word_x + CharWidth('#') * ((int) m_words[i].size())) &&
				   y >= word_y && y <= (word_y + (int) dy)) {
					//SendEvent(&app_main, EVT_TEXTBOX_POS, e->id, pos);
					m_pos = pos;
					draw();
					update();
					return;
				}

				pos += 1;
				width += m_words[i++].size();
			}
			yi++;
			width = 0;
		}

		if(x >= m_x + 4 && x <= m_x + m_w - 8 && y >= m_y + 4 && y <= m_y + m_h - 8) {
			SendEvent(&global_event_handler, EVT_TEXTBOX_POSITION, getID(), pos);
			m_pos = pos;
			draw();
			update();
		}
	}

	void onKeyUp(unsigned key) {
		if(key == KEY_LEFT) {
			if(m_pos == 0)
				m_pos = m_words.size();
			else
				m_pos--;
			draw();
			update();
		}
		else if(key == KEY_RIGHT) {
			if(m_pos == m_words.size())
				m_pos = 0;
			else
				m_pos++;
			draw();
			update();
		}
	}

	unsigned m_pos;
	vector<string> m_words;
	bool m_drawCursor;
};

/* ************************ Text view *************************************** */

class TextView : public Widget {
public:
	TextView(const char* text) : Widget() {
		m_text = string(text);
	}
	
	void draw() const {
		SetFont(const_cast<ifont*>(getFont()), BLACK);
		FillArea(m_x, m_y, m_w, m_h, WHITE);
		
		DrawTextRect(m_x, m_y, m_w, m_h, m_text.c_str(), ALIGN_LEFT);
	}
	
	unsigned getMinWidth() const {
		return CharWidth('#') * sqrt(m_text.length()) + 8 + 10;
	}
	
	unsigned getMinHeight() const {
		return getMinWidth();
	}
protected:
	void onTouchDown(unsigned, unsigned) {}
	void onKeyUp(unsigned) {}
private:
	string m_text;
};

/* *** Simple (no submenu) wrapper for built-in inkview menu  *************** */
class Menu {
public:
	Menu() {
		m_menu[0].type = 0;
		m_menu[0].index = 0;
		m_menu[0].text = 0;
		m_menu[0].submenu = 0;

		m_menu_size = 0;
		m_menu_id = m_menu_last_id;
		m_menu_last_id++;
	}

	void append(uint i_type, uint i_id, const char* i_label) {
		if(i_label != NULL)
			strcpy(m_menu_labels[m_menu_size], i_label);

		m_menu[m_menu_size].type = i_type;
		m_menu[m_menu_size].index = i_id + 100 * m_menu_id;

		if(i_label != NULL)
			m_menu[m_menu_size].text = m_menu_labels[m_menu_size];
		else
			m_menu[m_menu_size].text = NULL;

		m_menu[m_menu_size].submenu = NULL;
		m_menu_size++;
		m_menu[m_menu_size].type = 0;
		m_menu[m_menu_size].index = 0;
		m_menu[m_menu_size].text = NULL;
		m_menu[m_menu_size].submenu = NULL;
	}

	void setActive(uint id, bool active) {
		for(uint i = 0; i < m_menu_size; i++) {
			if(m_menu[i].index - 100 * m_menu_id == id) {
				m_menu[i].type = ((active) ? ITEM_ACTIVE : ITEM_INACTIVE);
				return;
			}
		}
	}

	void show() {
		show(ScreenWidth() / 2 - 50, ScreenHeight() / 2 - 100);
	}

	uint getID() {
		return m_menu_id;
	}

	void show(int x, int y) {
		if(m_menu_size == 0)
			return;

		OpenMenu(m_menu, 0, x, y, &menuCallback);
	}

private:
	static void menuCallback(int index) {
		SendEvent(&global_event_handler, EVT_MENU_SELECT, index / 100, index % 100);
	}

	/* TODO: make dyn array, now 16 items max*/
	imenu m_menu[16];
	char m_menu_labels[32][16];
	uint m_menu_size;
	uint m_menu_id;
	static uint m_menu_last_id;
};

uint Menu::m_menu_last_id = 0;

/* ************ Inkview's list wrapper  ************************************* */

/* TODO: split model\view or even make real MVC */
class FullscreenList {
public:
	FullscreenList(const char* title) {
		m_title = title;
		m_items = new vector<string>();
		m_id = m_last_id;
		m_last_id++;
		m_selected = -1;
	}

	uint getID() {
		return m_id;
	}

	void append(const char* label) {
		m_items->push_back(label);
		if(m_visible_list)
			show();
	}

	void remove(uint index) {
		m_items->erase(m_items->begin() + index);
		if(m_visible_list)
			show();
	}

	void clear() {
		m_items->clear();
	}

	void edit(uint index, const string& new_value) {
		m_items->at(index) = new_value;
		if(m_visible_list)
			show();
	}

	uint getSelected() const {
		return (uint) m_selected;
	}

	void show() {
		m_visible_list = this;
		OpenList(m_title.c_str(), NULL,
		         ScreenWidth() - 2 * c_wpad,
		         GetThemeFont("menu.font.normal", "")->height + c_hpad,
				 //OpenList doesn't work when number of items is 0,
				 //so using this hack
		         ((m_items->size() == 0) ? 1 : m_items->size()),
		         0, &listCallback
		        );
	}
	
	void hide() {
		m_visible_list = NULL;
		SetEventHandler(&global_event_handler);
	}

	~FullscreenList() {
		delete m_items;
	}

private:
	static int listCallback(int action, int x, int y, int idx, int state) {
		static ifont* menu_font = NULL;
		if(menu_font == NULL) {
			menu_font = GetThemeFont("menu.font.normal", "");
		}

		if(action == LIST_PAINT && m_visible_list->m_items->size() != 0) {
			SetFont(menu_font, menu_font->size);
			
			DrawTextRect(
				x + 2 * c_wpad,
				y,
				ScreenWidth() - 4 * c_wpad,
				menu_font->height + c_hpad / 2,
				m_visible_list->m_items->at(idx).c_str(),
				0
			);
			
			if(state == 1) {
				DrawSelection(x, y, ScreenWidth() - 2 * c_wpad, menu_font->height + c_hpad, BLACK);
			}
		}
		else if(action == LIST_EXIT) {
			m_visible_list = NULL;
			/* TODO: Am i doing it right? */
			SetEventHandler(&global_event_handler);
			return 1;
		}

		if(idx != -1 && state == 1 && m_visible_list->m_items->size() != 0)
			m_visible_list->m_selected = idx;

		if(m_visible_list->m_items->size() == 0 && action == LIST_MENU) {
			SendEvent(&global_event_handler, EVT_LIST_ACTION, m_visible_list->m_id*1000 + 999, LIST_MENU);
			return 1;
		}

		if(idx != -1 && m_visible_list->m_items->size() != 0) {
			//SendEvent(&global_event_handler, EVT_LIST_ACTION, (m_visible_list->m_id << 16) | idx, action);
			SendEvent(&global_event_handler, EVT_LIST_ACTION, m_visible_list->m_id*1000 + idx, action);
		}
		return 0;
	}

	string m_title;
	uint m_id;
	int m_selected;
	static const uint c_wpad = 16;
	static const uint c_hpad = 4;
	vector<string>* m_items;
	static FullscreenList* m_visible_list; //FIXME: this looks like a hack (or doesnt?)
	static uint m_last_id; //TODO: create an interface for classes that need IDs or derive them from Widget
};

FullscreenList* FullscreenList::m_visible_list = NULL;
uint FullscreenList::m_last_id = 0;

/* ********************** Keyboard input class ****************************** */

class Keyboard {
public:
	static void show(const string& title, const string& initText, uint id) {
		m_callerID = id;
		strcpy(m_kbdBuffer, initText.c_str());
		OpenKeyboard(const_cast<char*>(title.c_str()), m_kbdBuffer, sizeof(m_kbdBuffer) - 1, 0, &callback);
	}

	static string getText() {
		return string(m_kbdBuffer);
	}
private:
	static void callback(char* b) {
		if(b == NULL)
			SendEvent(&global_event_handler, EVT_KEYBOARD, m_callerID, -1);
		else
			SendEvent(&global_event_handler, EVT_KEYBOARD, m_callerID, 0);
	}
	//TODO: allow to change buffer size
	static char m_kbdBuffer[512];
	static uint m_callerID;
};

uint Keyboard::m_callerID;
char Keyboard::m_kbdBuffer[512];

/* *************** Main application class *********************************** */

double fparser_deg(const double* rad) {
	double result = 180 * rad[0] / M_PI;
	return result - (int (result / (2 * 180)) * 2 * 180);
}

double fparser_rad(const double* deg) {
	double result = M_PI * deg[0] / 180;
	return result - (int (result / (2 * M_PI)) * 2 * M_PI);
}

class Application {
public:
	Application() {
		setlocale(LC_NUMERIC, "C"); //force decimal separator character to '.'

		readConfig();
		initParser();

		m_variables = new double[5]; //ans, a, b, c, d
		m_variables[0] = 0.0;
		m_variables[1] = 0.0;
		m_variables[2] = 0.0;
		m_variables[3] = 0.0;
		m_variables[4] = 0.0;

		m_textboxFont = OpenFont(CFG_FONT_NAME, CFG_TEXTBOX_FONT_SIZE, 0);

		if(ScreenWidth() <= 600)
			Widget::setGlobalFont(OpenFont(CFG_FONT_NAME, CFG_BUTTON_FONT_SIZE - 4, 0));
		else
			Widget::setGlobalFont(OpenFont(CFG_FONT_NAME, CFG_BUTTON_FONT_SIZE, 0));

		m_inputBox = new TextBox();
		m_inputBox->setPos(c_padding, c_padding);
		m_inputBox->setSize(ScreenWidth() - c_padding * 2, c_input_height);
		m_inputBox->setFont(m_textboxFont);
		m_inputBox->setDrawCursor(true);

		Widget::setFocus(m_inputBox, false);

		m_buttonCalculate = new Button("=");
		m_buttonCalculate->setPos(c_padding + ScreenWidth() - c_padding * 2 - c_answer_height * 2 - c_padding,
		                          m_inputBox->getY() + m_inputBox->getHeight() + c_padding);
		m_buttonCalculate->setSize(c_answer_height, c_answer_height);

		m_buttonClear = new Button("←");
		m_buttonClear->setPos(c_padding + ScreenWidth() - c_padding * 2 - c_answer_height,
		                      m_inputBox->getY() + m_inputBox->getHeight() + c_padding);
		m_buttonClear->setSize(c_answer_height, c_answer_height);

		m_answerBox = new TextBox();
		m_answerBox->setPos(c_padding, m_inputBox->getY() + m_inputBox->getHeight() + c_padding);
		m_answerBox->setSize(ScreenWidth() - c_padding * 2 - c_answer_height * 2 - c_padding * 2, c_answer_height);
		m_answerBox->setDrawCursor(false);
		m_answerBox->setFont(m_textboxFont);

		m_menu = new Menu();
		m_menu->append(ITEM_HEADER, 0, "Calculator");
		m_menu->append(ITEM_ACTIVE, c_menu_eval, "Keyboard");
		m_menu->append(ITEM_ACTIVE, c_menu_custom, "Expressions");
		m_menu->append(ITEM_ACTIVE, c_menu_history, "History");
		m_menu->append(ITEM_ACTIVE, c_menu_help, "Help");
		m_menu->append(ITEM_SEPARATOR, 0, NULL);
		m_menu->append(ITEM_ACTIVE, c_menu_exit, "Exit");

		m_listMenu = new Menu();
		m_listMenu->append(ITEM_HEADER, 0, "Edit item");
		m_listMenu->append(ITEM_ACTIVE, c_menu_list_add, "Add");
		m_listMenu->append(ITEM_ACTIVE, c_menu_list_edit, "Edit");
		m_listMenu->append(ITEM_ACTIVE, c_menu_list_remove, "Remove");

		m_exprList = new FullscreenList("Defined expressions");
		for(uint i = 0; i < m_customExpr.size(); i++)
			m_exprList->append(m_customExpr[i].first.c_str());

		m_historyList = new FullscreenList("History");
		for(uint i = 0; i < m_history.size(); i++) {
			string hist_ent;

			for(uint j = 0; j < m_history[i].size(); j++)
				hist_ent += m_history[i][j];
			m_historyList->append(hist_ent.c_str());
		}

		m_buttonsLayout = new GridLayout(
		    c_padding,
		    m_answerBox->getY() + m_answerBox->getHeight() + c_padding,
		    CFG_GRID_ROWS,
		    CFG_GRID_COLS,
		    ScreenWidth() - c_padding * 2,
		    ScreenHeight() - c_padding * 2 - m_answerBox->getY() - m_answerBox->getHeight() - c_padding,
		    c_layout_sp
		);
		
		m_helpView = new TextView(c_help_msg);
		m_helpView->setPos(c_padding, c_padding);
		m_helpView->setSize(ScreenWidth() - 2 * c_padding,
							ScreenHeight() - 2 * c_padding);
		m_helpView->setVisibility(false);

		for(unsigned row = 0; row < CFG_GRID_ROWS; row++) {
			for(unsigned col = 0; col < CFG_GRID_COLS; col++) {
				if(functions[row][col].name[0] != '\0') {
					Button* tmp = NULL;
					if(functions[row][col].is_regular_button)
						tmp = new Button(functions[row][col].name);
					else
						tmp = new CalcButton(&functions[row][col]);
					m_buttonsLayout->insert(tmp, row, col);
				}
			}
		}
		
		m_buttonExpr = static_cast<Button*>(Widget::findByName("expr"));
		
		m_buttonsLayout->update();

		ClearScreen();
	}

	~Application() {
		delete m_inputBox;
		delete m_answerBox;
		delete m_buttonCalculate;
		delete m_buttonClear;

		for(unsigned row = 0; row < CFG_GRID_ROWS; row++) {
			for(unsigned col = 0; col < CFG_GRID_COLS; col++) {
				if((*m_buttonsLayout) [row][col])
					delete(*m_buttonsLayout) [row][col];
			}
		}

		writeConfig();

		delete m_menu;
		delete m_listMenu;
		delete m_exprList;
		delete m_historyList;
		delete m_buttonsLayout;
		delete m_helpView;
		CloseFont(const_cast<ifont*>(Widget::getGlobalFont()));
		CloseFont(m_textboxFont);
		delete m_fparser;
		delete [] m_variables;
	}

	void redraw() const {
		ClearScreen();
		Widget::drawAll();
		FullUpdate();
	}

	int event(int event, int param1, int param2) {
		switch(event) {
			case EVT_SHOW:
				redraw();
				break;
			case EVT_CALC_BUTTON_ACTIVATE: 
				__DBG("got event from calc button")
				onCalcButtonPressed(param1);
			break;
			case EVT_BUTTON_ACTIVATE:
				__DBG("got event from button")
				onButtonPressed(param1, param2);
			break;
			case EVT_LIST_ACTION:
				onListItemActivated(param1, param2);
			break;
			case EVT_MENU_SELECT:
				onMenuSelect(param1, param2);
				break;
			case EVT_KEYBOARD:
				onKeyboard(param1, param2);
				break;
			case EVT_KEYPRESS:
				if(m_helpView->getVisibility() == true) {
					Widget::showAll();
					m_helpView->setVisibility(false);
					redraw();
					break;
				}
				
				if(param1 == KEY_DOWN) {
					if(!moveFocus('d'))
						break;
				}
				else if(param1 == KEY_UP) {
					if(!moveFocus('u'))
						break;
				}
				else if(param1 == KEY_LEFT) {
					if(!moveFocus('l'))
						break;
				}
				else if(param1 == KEY_RIGHT) {
					if(!moveFocus('r'))
						break;
				}
				else if(param1 == KEY_MENU) {
					m_menu->show();
					break;
				}
			case EVT_KEYRELEASE:
			case EVT_POINTERDOWN:
			case EVT_POINTERUP:
			case EVT_POINTERLONG:
				Widget::processEvent(event, param1, param2);
				break;
			default:
				break;
		}

		return 0;
	}

private:
	void onCalcButtonPressed(int id) {
		CalcButton* btn = static_cast<CalcButton*>(Widget::getByID(id));
		__DBG("found btn" << btn);
		unsigned r, c;
		for(r = 0; r < CFG_GRID_ROWS; r++)
			for(c = 0; c < CFG_GRID_COLS; c++) {
				if((*m_buttonsLayout) [r][c] == btn)
					goto found_widget;
			}
		found_widget:
		m_focusedBtnRow = r;
		m_focusedBtnCol = c;
		__DBG("got row and col" << r << " " << c)
		__DBG("btn func: " << btn->getFunc()->str[0])
		unsigned i = 0;
		while(btn->getFunc()->str[i][0] != '\0') {
			__DBG("iter " << i)
			m_inputBox->insert(string(btn->getFunc()->str[i++]), m_inputBox->getTextPos());
			__DBG("\tinsert ok")
			m_inputBox->setTextPos(m_inputBox->getTextPos() + 1);
			__DBG("\ttext pos ok")
		}
		__DBG("input box pos set")
		m_inputBox->setTextPos(m_inputBox->getTextPos() - btn->getFunc()->pos);
		m_inputBox->draw();
		m_inputBox->update();
		__DBG("done")
	}
	
	void onButtonPressed(int id, int long_press) {
		if((unsigned) id == m_buttonClear->getID() && long_press) {
			m_inputBox->clear();
			m_inputBox->update();
		}
		else if((unsigned) id == m_buttonClear->getID() && !long_press) {
			if(m_inputBox->getTextPos() != 0) {
				m_inputBox->words().erase(m_inputBox->words().begin() + m_inputBox->getTextPos() - 1);
				m_inputBox->setTextPos(m_inputBox->getTextPos() - 1);
				m_inputBox->draw();
				m_inputBox->update();
			}
		}
		else if((unsigned) id == m_buttonCalculate->getID()) {
			bool result = evalAndDisplay(m_inputBox->getString());
			if(result)
				historyAppend(m_inputBox->words());
			
			m_inputBox->clear();
			m_inputBox->draw();
			m_inputBox->asyncUpdate();
		}
		else if((unsigned) id == m_buttonExpr->getID()) {
			m_exprList->show();
		}
	}
	
	void onListItemActivated(int partid, uint action) {
		if(action == LIST_MENU) {
			if((partid % 1000) == 999) {      //no items are selected
				m_listMenu->setActive(c_menu_list_edit, false);
				m_listMenu->setActive(c_menu_list_remove, false);
			}
			
			m_listMenu->show();
			
			m_listMenu->setActive(c_menu_list_edit, true);
			m_listMenu->setActive(c_menu_list_remove, true);
		}
		else if(action == LIST_OPEN) {
			if((uint)partid/1000 == m_exprList->getID()) {
				__DBG("expr")
				uint index = partid % 1000;
				string name1;
				string::iterator lbrace = std::find(m_customExpr[index].first.begin(),
													m_customExpr[index].first.end(),
													'(');
				
				std::copy(
					m_customExpr[index].first.begin(),
					lbrace,
					std::back_inserter(name1)
				);
				
				name1.push_back('(');
				
				m_inputBox->insert(name1, m_inputBox->getTextPos());
				m_inputBox->insert(")", m_inputBox->getTextPos() + 1);
				m_inputBox->asyncUpdate();
				m_inputBox->setTextPos(m_inputBox->getTextPos() + 1);
				
				m_exprList->hide();
			}
			else if((uint)partid/1000 == m_historyList->getID()) {
				__DBG("history")
				uint index = partid % 1000;
				m_inputBox->clear();
				
				for(vector<string>::iterator it = m_history[index].begin(); it != m_history[index].end(); it++)
					m_inputBox->append(*it);
				
				m_inputBox->setTextPos(m_inputBox->words().size() - 1);
				m_inputBox->asyncUpdate();
				
				m_historyList->hide();
			}
		}
	}
	
	void onMenuSelect(int id, int item_id) {
		if(id == 0) {    //app menu
			switch(item_id) {
				case c_menu_exit:
					CloseApp();
					break;
				case c_menu_eval: {
					Keyboard::show("Enter an expression to evaluate", m_inputBox->getString(), c_menu_eval);
				}
				break;
				case c_menu_custom: {
					m_exprList->show();
				}
				break;
				case c_menu_history: {
					//rebuild history
					//TODO: cut long lines
					m_historyList->clear();
					__DBG(m_history.size())
					for(uint i = 0; i < m_history.size(); i++) {
						string hist_ent;
						//for_each(.. ,bind1st(mem_fun,..))
						for(uint j = 0; j < m_history[i].size(); j++)
							hist_ent.append(m_history[i][j]);
						m_historyList->append(hist_ent.c_str());
					}
					m_historyList->show();
				}
				break;
				case c_menu_help:
					Widget::hideAll();
					m_helpView->setVisibility(true);
					m_helpView->draw();
					m_helpView->update();
				break;
				default:
					break;
			}
		}
		else if(id == 1) {      //expr list menu
			switch(item_id) {
				case c_menu_list_add: {
					Keyboard::show("Add expression", "f(x,y) = x^y", c_menu_list_add);
				}
				break;
				case c_menu_list_edit: {
					Keyboard::show("Edit expression",
								   m_customExpr[m_exprList->getSelected()].first +
								   " = " +
								   m_customExpr[m_exprList->getSelected()].second,
								   c_menu_list_edit
					);
				}
				break;
				case c_menu_list_remove:
					m_customExpr.erase(m_customExpr.begin() + m_exprList->getSelected());
					m_exprList->remove(m_exprList->getSelected());
					break;
			}
		}
	}
	
	void onKeyboard(int caller, int action) {
		if(action == -1)    //input canceled
			return;
		
		if((uint) caller == c_menu_list_add) {
			try {
				string name, body, var;
				parseExpression(Keyboard::getText().c_str(), name, body, var);
				m_exprList->append(addExpression(name, body, var).c_str());
				m_customExpr.push_back(std::pair<string, string> (name, body));
			}
			catch(const string& s) {
				Message(ICON_ERROR, "Invalid expression", s.c_str(), 10);
			}
		}
		else if((uint) caller == c_menu_list_edit) {
			try {
				string name, body, var;
				parseExpression(Keyboard::getText(), name, body, var);
				m_customExpr[m_exprList->getSelected()] = std::pair<string, string> (name, body);
				m_exprList->edit(m_exprList->getSelected(), name);
				delete m_fparser;
				initParser();
			}
			catch(const string& s) {
				Message(ICON_ERROR, "Invalid expression", s.c_str(), 10);
			}
		}
		else if((uint) caller == c_menu_eval) {
			string kbd_str = Keyboard::getText();
			bool res = evalAndDisplay(kbd_str);
			if(!res)
				return;
			
			vector<string> hist_ent;
			for(string::iterator it = kbd_str.begin(); it != kbd_str.end(); it++)
				hist_ent.push_back(string(1, *it));
			historyAppend(hist_ent);
			
			m_inputBox->clear();
			m_inputBox->draw();
			m_inputBox->update();
		}
	}
	
	void initParser() {
		m_fparser = new FunctionParser();
		m_fparser->AddConstant("pi", M_PI);
		m_fparser->AddFunction("deg", &fparser_deg, 1);
		m_fparser->AddFunction("rad", &fparser_rad, 1);

		for(uint i = 0; i < m_customExpr.size(); i++) {
			string& name = m_customExpr[i].first;
			string variables;

			if(std::find(name.begin(), name.end(), '(') != name.end())
				std::copy(
				    std::find(name.begin(), name.end(), '(') + 1,
				    std::find(name.begin(), name.end(), ')'),
				    std::back_insert_iterator<string> (variables)
				);
			string s = addExpression(name, m_customExpr[i].second, variables);
		}
	}

	void readConfig() {
		iconfig* cfg = OpenConfig(c_config, NULL);

		char* history = NULL;
		vector<string> expr_vec, hist_vec;

		char* expressions = ReadString(cfg, "expressions", NULL);
		if(expressions == NULL)
			goto create_config;

		m_customExpr.clear();
		expr_vec = splitStr(expressions, ";");
		for(uint i = 0; i < expr_vec.size(); i++) {
			string name, body, variables;
			__DBG("push")
			parseExpression(expr_vec[i], name, body, variables);
			__DBG("parse done")
			m_customExpr.push_back(std::pair<string, string> (name, body));
		}
		__DBG("HISTORY")
		history = ReadString(cfg, "history", NULL);
		if(history == NULL)
			goto create_config;

		m_history.clear();
		hist_vec = splitStr(history, "#");
		for(uint i = 0; i < hist_vec.size(); i++) {
			vector<string> hist_ent = splitStr(hist_vec[i].c_str(), ";");
			m_history.push_back(hist_ent);
		}

	create_config:
		if(expressions == NULL) {
			WriteString(cfg, "expressions", "");
			WriteString(cfg, "history", "");
		}

		CloseConfig(cfg);
	}

	void writeConfig() {
		iconfig* cfg = OpenConfig(c_config, NULL);
		uint expr_str_size = 0;
		for(uint i = 0; i < m_customExpr.size(); i++) {
			// f(x) + " = " + body + '\n'
			expr_str_size += m_customExpr[i].first.size() + 3 + m_customExpr[i].second.size() + 1;
		}
		expr_str_size++;

		char* expressions = new char[expr_str_size];
		expressions[0] = '\0';
		for(uint i = 0; i < m_customExpr.size(); i++) {
			strcat(expressions, m_customExpr[i].first.c_str());
			strcat(expressions, " = ");
			strcat(expressions, m_customExpr[i].second.c_str());
			strcat(expressions, ";");
		}
		expressions[strlen(expressions) - 1] = '\0';
		WriteString(cfg, "expressions", expressions);

		delete [] expressions;

		uint hist_size = 0;
		for(uint i = 0; i < m_history.size(); i++) {
			for(uint j = 0; j < m_history[i].size(); j++) {
				hist_size += m_history[i][j].size() + 1;
			}
		}
		hist_size++;

		char* history = new char[hist_size];
		history[0] = '\0';
		for(uint i = 0; i < m_history.size(); i++) {
			for(uint j = 0; j < m_history[i].size(); j++) {
				strcat(history, m_history[i][j].c_str());
				strcat(history, ";");
			}
			strcat(history, "#");
		}
		history[strlen(history) - 1] = '\0';
		WriteString(cfg, "history", history);
		delete [] history;

		CloseConfig(cfg);
	}

	bool parseExpression(const string& expression, string& name, string& body, string& variables) {
		std::istringstream iss(expression);

		name = "";
		body = "";
		variables = "";

		std::getline(iss, name, '=');
		std::getline(iss, body);
		if(std::find(name.begin(), name.end(), '(') != name.end())
			std::copy(
			    std::find(name.begin(), name.end(), '(') + 1,
			    std::find(name.begin(), name.end(), ')'),
			    std::back_insert_iterator<string> (variables)
			);

		//trim spaces
		body.erase(
			body.begin(),
			std::find_if(
				body.begin(), body.end(),
				std::not1(std::ptr_fun<int, int> (std::isspace))
			)
		);
		name.erase(
			std::find_if(
				name.rbegin(), name.rend(),
				std::not1(std::ptr_fun<int, int> (std::isspace))).base(),
			name.end()
		);

		return true;
	}

	string addExpression(const string& name, const string& body, const string& variables) {
		static FunctionParser parser;

		parser.Parse(body, variables);
		if(parser.GetParseErrorType() != FunctionParser::FP_NO_ERROR) {
			throw string(parser.ErrorMsg());
		}
		string namepart;
		std::copy(name.begin(),
		          std::find(name.begin(),
		                    name.end(),
		                    '('),
		          std::back_inserter(namepart)
		         );
		if(!m_fparser->AddFunction(namepart, parser))
			throw string("Cannot add function");

		return name;
	}

	bool processAssignment(const string& expression) {
		std::istringstream iss(expression);
		
		string name = "";
		string body = "";
		const string assign_op = ":=";
		
		std::size_t pos = expression.find(assign_op);
		if(pos != std::string::npos) {
			name = expression.substr(0, pos);
			body = expression.substr(pos + assign_op.length());
		} else {
			return false;
		}
		
		name.erase(
			std::find_if(
				name.rbegin(), name.rend(),
				std::not1(std::ptr_fun<int, int> (std::isspace))).base(),
			name.end()
		);
		
		string variables = "abcd";
		size_t var_index = name[0]  - 'a' + 1;
		__DBG("assign var idx is " << var_index);
		__DBG("assign var is " << name[0]);
		__DBG("assign body is " << body);
		if(variables.find(name) != std::string::npos) {
			double value = evalExpression(body);
			__DBG("assign prev value is " << m_variables[var_index]);
			m_variables[var_index] = value;
			__DBG("assign value is " << m_variables[var_index]);
		}
		else {
			throw string("Assigned variable error");
		}
		
		return true;
	}
	
	double evalExpression(const string& expression) {
		//TODO: automatical variable appending
		int parser_result = m_fparser->Parse(expression, "ans,a,b,c,d");
		double result = m_fparser->Eval(m_variables);

		if(parser_result != -1)
			throw string(m_fparser->ErrorMsg());

		if(m_fparser->EvalError() != 0)
			throw string("Evaluation error");

		return result;
	}

	void historyAppend(const vector<string>& item) {
		if(m_history.size() > c_history_size)
			m_history.pop_back();
		m_history.push_front(item);
	}

	bool evalAndDisplay(const string& expression) {
		m_answerBox->words().clear();

		double result = 0.0;
		try {
			bool has_assign = processAssignment(expression);
			if(has_assign)
				return true;
			
			result = evalExpression(expression);
		}
		catch(const string& errMsg) {
			m_answerBox->words().push_back(errMsg);
			m_answerBox->draw();
			m_answerBox->asyncUpdate();
			return false;
		}

		static std::ostringstream ost;
		ost.clear();
		ost.str("");
		ost << result;
		m_answerBox->words().push_back(ost.str());
		m_variables[0] = result; //save result to 'ans' variable

		m_answerBox->draw();
		m_answerBox->asyncUpdate();
		return true;
	}

	bool moveFocus(char dir) {
		switch(dir) {
			case 'd': //down
				if(Widget::getFocus() == m_inputBox) {
					focusAndUpdate(m_buttonCalculate);
				}
				else if(Widget::getFocus() == m_buttonCalculate ||
				        Widget::getFocus() == m_buttonClear) {
					m_focusedBtnRow = 0;
					m_focusedBtnCol = 0;
					focusAndUpdate((*m_buttonsLayout) [0][0]);
				}
				else {
					unsigned nextCell = gridColNextCell(m_focusedBtnRow, true);
					if(nextCell == m_focusedBtnRow)
						focusAndUpdate(m_inputBox);
					else {
						m_focusedBtnRow = nextCell;
						focusAndUpdate((*m_buttonsLayout) [nextCell][m_focusedBtnCol]);
					}
				}
				break;
			case 'u': //up
				if(Widget::getFocus() == m_inputBox) {
					m_focusedBtnRow = CFG_GRID_ROWS - 1;
					m_focusedBtnCol = 0;
					focusAndUpdate((*m_buttonsLayout) [m_focusedBtnRow][m_focusedBtnCol]);
				}
				else if(Widget::getFocus() == m_buttonCalculate ||
				        Widget::getFocus() == m_buttonClear) {
					focusAndUpdate(m_inputBox);
				}
				else {
					unsigned nextCell = gridColNextCell(m_focusedBtnRow, false);
					if(nextCell == m_focusedBtnRow) {
						focusAndUpdate(m_buttonClear);
					}
					else {
						m_focusedBtnRow = nextCell;
						focusAndUpdate((*m_buttonsLayout) [nextCell][m_focusedBtnCol]);
					}
				}
				break;
			case 'l': //left
				if(Widget::getFocus() == m_inputBox) {
					return true;
				}
				else if(Widget::getFocus() == m_buttonCalculate) {
					focusAndUpdate(m_inputBox);
				}
				else if(Widget::getFocus() == m_buttonClear) {
					focusAndUpdate(m_buttonCalculate);
				}
				else {
					unsigned nextCell = gridRowNextCell(m_focusedBtnCol, false);
					if(nextCell == m_focusedBtnCol) {
						m_focusedBtnCol = gridRowNextCell(CFG_GRID_COLS, false);
						focusAndUpdate((*m_buttonsLayout) [m_focusedBtnRow][m_focusedBtnCol]);
					}
					else {
						m_focusedBtnCol = nextCell;
						focusAndUpdate((*m_buttonsLayout) [m_focusedBtnRow][nextCell]);
					}
				}
				break;
			case 'r': //right
				if(Widget::getFocus() == m_inputBox) {
					return true;
				}
				else if(Widget::getFocus() == m_buttonCalculate) {
					focusAndUpdate(m_buttonClear);
				}
				else if(Widget::getFocus() == m_buttonClear) {
					m_focusedBtnRow = 0;
					m_focusedBtnCol = 0;
					focusAndUpdate((*m_buttonsLayout) [m_focusedBtnRow][m_focusedBtnCol]);
				}
				else {
					unsigned nextCell = gridRowNextCell(m_focusedBtnCol, true);
					if(nextCell == m_focusedBtnCol) {
						m_focusedBtnCol = gridRowNextCell(-1, true);
						focusAndUpdate((*m_buttonsLayout) [m_focusedBtnRow][m_focusedBtnCol]);
					}
					else {
						m_focusedBtnCol = nextCell;
						focusAndUpdate((*m_buttonsLayout) [m_focusedBtnRow][nextCell]);
					}
				}
				break;
		}

		return false;
	}

	static vector<string> splitStr(string str, const char* delim) {
		vector<string> result;
		char* str_data = new char[str.size() + 1];
		strcpy(str_data, str.c_str());
		char* part = strtok(str_data, delim);
		while(part != NULL) {
			result.push_back(part);
			part = strtok(NULL, delim);
		}
		delete [] str_data;
		return result;
	}

	void focusAndUpdate(Widget* w) {
		if(w->getVisibility()) {
			Widget::setFocus(w, true);
			w->draw();
			w->update();
		}
	}

	unsigned gridRowNextCell(int col, bool dir_right) {
		int c = col + ((dir_right) ? 1 : -1);

		while(c < CFG_GRID_COLS && c >= 0 && (*m_buttonsLayout) [m_focusedBtnRow][c] == NULL)
			(dir_right) ? c++ : c--;
		if(c == CFG_GRID_COLS || c < 0)
			return col;

		return c;
	}

	unsigned gridColNextCell(int row, bool dir_down) {
		int r = row + ((dir_down) ? 1 : -1);

		while(r < CFG_GRID_ROWS && r >= 0 && (*m_buttonsLayout) [r][m_focusedBtnCol] == NULL)
			(dir_down) ? r++ : r--;
		if(r == CFG_GRID_ROWS || r < 0)
			return row;

		return r;
	}

	static const unsigned c_padding = 10;
	static const unsigned c_input_height = 200;
	static const unsigned c_answer_height = 30;
	static const unsigned c_layout_sp = 10;

	static const uint c_menu_eval = 1;
	static const uint c_menu_exit = 2;
	static const uint c_menu_custom = 3;
	static const uint c_menu_history = 4;
	static const uint c_menu_help = 5;

	static const uint c_menu_list_add = 5;
	static const uint c_menu_list_remove = 6;
	static const uint c_menu_list_edit = 7;

	static const uint c_history_size = 20;

	static const char c_config[];
	static const char c_help_msg[];
	
	unsigned m_focusedBtnRow;
	unsigned m_focusedBtnCol;
	double* m_variables;

	TextView* m_helpView;
	Menu* m_menu;
	Menu* m_listMenu;
	FullscreenList* m_exprList;
	FullscreenList* m_historyList;
	ifont* m_textboxFont;
	GridLayout* m_buttonsLayout;
	TextBox* m_inputBox;
	TextBox* m_answerBox;
	Button* m_buttonCalculate;
	Button* m_buttonClear;
	Button* m_buttonExpr;
	FunctionParser* m_fparser;
	vector<std::pair<string, string> > m_customExpr;
	deque<vector<string> > m_history;
};

const char Application::c_config[] = CONFIGPATH "/ecalc.cfg";
const char Application::c_help_msg[] =
	"HELP (Press any key to exit)\n\n"
	"MAIN SCREEN BUTTONS\n"
	"    * \" = \" evaluate and show result\n"
	"    * \" ← \" works like Backspace key on short press,\n"
	"              and clears all on long press\n"
	"    * \" . \" (dot) inserts decimal separator\n"
	"    * \" :=  \" is assignment operator\n"
	"                (syntax: <VARIABLE>:=<EXPRESSION>)\n"
	"    * \" E \" inserts \"*10^()\"\n"
	"    * \" exp \" is exponential function\n"
	"    * \" pi \" inserts Pi constant\n"
	"    * \" expr \" shows a list of user-defined functions\n"
	"    * \" rad \", \" deg \" radians <-> degrees conversion\n"
	"                           functions \n"
	"    * \" abs \" is absolute value function\n"
	"    * \" ans \" inserts variable that holds previous answer\n"
	"    * \" , \" is used as function parameters separator\n"
	"    * \"a\",\"b\",\"c\",\"d\" preset variable names to use\n"
	"                              with \":=\" operator\n"
	"MENU ITEMS\n"
	"    * \"Keyboard\" - enter an expresion using\n"
	"                     standart keyboard\n"
	"    * \"Expressions\" - show user-defined expressions\n"
	"                        you can add, edit, and delete\n"
	"                        them using popup menu\n"
	"    * \"History\" - history of entered expressions\n"
	"    * \"Help\" - this help\n"
	"    * \"Exit\" - guess what?\n";
	
/* ********************** Event loop and main() ***************************** */

Application* app = NULL;

int global_event_handler(int type, int param1, int param2) {
	switch(type) {
		case EVT_INIT:
			app = new Application();
			break;
		case EVT_SHOW:
			app->redraw();
			break;
		case EVT_EXIT:
			delete app;
			break;
		case EVT_KEYPRESS:
			if(param1 == KEY_BACK)
				CloseApp();
		default:
			app->event(type, param1, param2);
			break;
	}

	return 0;
}

int main(int, const char**) {

	InkViewMain(&global_event_handler);

	return 0;
}
