/**
* \file GUI_objects.c
* \brief BRTOS LCD Graphical Objects
*
* Author: Gustavo Weber Denardin and Miguel Moreto
*
*
**/

#include "GUI_objects.h"
#include "touch_7846.h"
#include "utils.h"

/* Declare object linked list head and tail */
ObjectEvent_t *ObjectHead;
ObjectEvent_t *ObjectTail;

/* Declare GLCD mutex */
BRTOS_Mutex	*GUIMutex;

/* Declare touchscreen events list */
BRTOS_Queue	*TouchEvents;

/* Declare touchscreen events signaling semaphore */
BRTOS_Sem 	*TouchSync;

/* Declare background color */
color_t GuiBackground;

/* Declare Button functions structure */
ButtonFunc_t Button;

/* Declare Slider functions structure */
SliderFunc_t Slider;

/* Declare Checkbox functions structure */
CheckboxFunc_t Checkbox;

/* Declare Graph functions structure */
GraphFunc_t Graph;

/*  Initialization of the functions structures */
void GUI_ObjetcsInit(color_t background, unsigned char mutex_prio)
{
#if (USE_BUTTON == TRUE)
	Button.Init = Button_Init;
	Button.Draw = Button_Draw;
	Button.Update = Button_Update;
	Button.Click = Button_Click;
#endif

#if (USE_SLIDER == TRUE)
	Slider.Init = Slider_Init;
	Slider.Draw = Slider_Draw;
	Slider.Update = Slider_Update;
	Slider.Click = Slider_Click;
#endif

#if (USE_CHECKBOX == TRUE)
	Checkbox.Init = Checkbox_Init;
	Checkbox.Draw = Checkbox_Draw;
	Checkbox.Update = Checkbox_Update;
	Checkbox.Click = Checkbox_Click;
#endif

#if (USE_GRAPH == TRUE)
	Graph.Init = Graph_Init;
	Graph.Draw = Graph_Draw;
	Graph.AddTraceData = Graph_AddTraceData;
#endif

	GuiBackground = background;

	// Create GUI mutex
	if(OSMutexCreate(&GUIMutex,mutex_prio) != ALLOC_EVENT_OK)
	{
		// Fail
	}
}


/* Declare function used to include objects into the object event list */
void GUI_IncludeObjectIntoEventList(ObjectEvent_t *object)
{
	if (ObjectTail != NULL)
	{
		ObjectTail->Next = object;
		object->Previous = ObjectTail;
		ObjectTail = object;
		ObjectTail->Next = NULL;
	}
	else
	{
		ObjectTail = object;
		ObjectHead = object;
		object->Next = NULL;
		object->Previous = NULL;
	}
}


/* Declare function used to remove objects from the object event list */
void GUI_RemoveObjectIntoEventList(ObjectEvent_t *object)
{
	if (object == ObjectHead)
	{
		if (object == ObjectTail)
		{
			ObjectTail = NULL;
			ObjectHead = NULL;
		}else
		{
			ObjectHead = object->Next;
			ObjectHead->Previous = NULL;
		}
	}else
	{
		if (object == ObjectTail)
		{
			ObjectTail = object->Previous;
			ObjectTail->Next = NULL;
		}else
		{
			object->Next->Previous = object->Previous;
			object->Previous->Next = object->Next;
		}
	}
}



/* Declare function used to verify objects inside touched region */
ObjectEvent_t *GUI_VerifyObjects(int x, int y)
{
	ObjectEvent_t *object = ObjectHead;

	  ////////////////////////////////////////////////////
	  // Search for inside object 						//
	  ////////////////////////////////////////////////////
	  while(object != NULL)
	  {
		  if((x >= (object->x1)) && (x <= (object->x2)) && (y >= (object->y1)) && (y <= (object->y2)))
		  {
			  return object;
		  }else
		  {
			  object = object->Next;
		  }
	  }
	  return NULL;
}


/* Declare GUI event handler task */
void GUI_Event_Handler(void *param)
{
	Touch_t 			TouchPos;
	ObjectEvent_t 		*object;
	Button_t			*button;
	Slider_t			*slider;
	Checkbox_t			*checkbox;
	TouchPos.x = 0;
	TouchPos.y = 0;

	(void)OSDQueueCreate(1024, sizeof(Callbacks), &TouchEvents);
	(void)OSSemCreate(0,&TouchSync);

	while(1)
	{
		// Wait for a touch in the screen
		(void)OSSemPend(TouchSync,0);

		// Read touch position
		TP_Read();

		// If there is touch in a different place
		// !!!!!!!!!!!! Lembrar que esse if é interessante para uma tela de paint
		//if ((TouchPos.x != Pen_Point.X0) || (TouchPos.y != Pen_Point.Y0))
		//{
			TouchPos.x = Pen_Point.X0;
			TouchPos.y = Pen_Point.Y0;

			object = GUI_VerifyObjects(TouchPos.x, TouchPos.y);
			if (object != FALSE)
			{
				switch(object->object)
				{
					case BUTTON_OBJECT:
						button = (Button_t*)object->ObjectPointer;
						Button.Click(button);
						while(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0)
						{
							DelayTask(10);
						}
						OSDQueuePost(TouchEvents, (void*)(&(button->ClickEvent)));
						Button.Draw(button);
					break;

					case SLIDER_OBJECT:
						slider = (Slider_t*)object->ObjectPointer;
						int slider_size = (slider->x2);
						while(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0)
						{
							DelayTask(20);

							// Read touch position
							TP_Read();
							TouchPos.x = Pen_Point.X0;
							TouchPos.y = Pen_Point.Y0;

							// Verify if still over slider
							if (object == GUI_VerifyObjects(TouchPos.x, TouchPos.y))
							{
								// Calcule new value
								slider->value = (((TouchPos.x) - (slider->x1)) * 100) / slider_size;

								if (slider->value <0) slider->value = 0;
								if (slider->value >100) slider->value = 100;

								// Draw slider bar
								Slider.Click(slider);
							}
						}
						OSDQueuePost(TouchEvents, (void*)(&(slider->ClickEvent)));
					break;

					case CHECKBOX_OBJECT:
						checkbox = (Checkbox_t*)object->ObjectPointer;

						// New value
						if (checkbox->value == TRUE)
						{
							checkbox->value = FALSE;
						}else
						{
							checkbox->value = TRUE;
						}
						Checkbox.Click(checkbox);
						while(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0)
						{
							DelayTask(10);
						}
						OSDQueuePost(TouchEvents, (void*)(&(checkbox->ClickEvent)));
					break;

					/* Unknown object */
					default:
					break;
				}
			}
		//}
		DelayTask(3);

	    // Enable interrupt again.
	    if(EXTI_GetITStatus(TOUCH_EXTI_Line) != RESET)
	    {
	    	EXTI_ClearITPendingBit(TOUCH_EXTI_Line);
	    }
	    TP_InterruptEnable(ENABLE);
	}
}

/* Touchscreen interrupt handler */
void TP_Handler(void){

	(void)OSSemPost(TouchSync);

	/* Disable TP interrupt: */
	TP_InterruptEnable(DISABLE);
}




#if (USE_BUTTON == TRUE)
/* Initializes the Button structure */
void Button_Init(coord_t x, coord_t y, coord_t width, coord_t height,
		coord_t radius, color_t bg_color, color_t font_color, char *str,
		Button_t *Button_struct, Callbacks click_event)
{
	Button_struct->x = x;
	Button_struct->y = y;
	Button_struct->dx = width;
	Button_struct->dy = height;
	Button_struct->radius = radius;
	Button_struct->bg_color = bg_color;
	Button_struct->font_color = font_color;
	Button_struct->str = str;

	// Event handler
	Button_struct->event.x1 = x+2;
	Button_struct->event.y1 = y+2;
	Button_struct->event.x2 = x+width-4;
	Button_struct->event.y2 = y+height-4;

	GUI_IncludeObjectIntoEventList(&(Button_struct->event));

	Button_struct->event.object = BUTTON_OBJECT;
	Button_struct->event.ObjectPointer = (void*)Button_struct;
	Button_struct->ClickEvent = click_event;
}


/* Initializes the Button structure */
void Button_Update(coord_t x, coord_t y, coord_t width, coord_t height,
		coord_t radius, color_t bg_color, color_t font_color, char *str,
		Button_t *Button_struct)
{
	Button_struct->x = x;
	Button_struct->y = y;
	Button_struct->dx = width;
	Button_struct->dy = height;
	Button_struct->radius = radius;
	Button_struct->bg_color = bg_color;
	Button_struct->font_color = font_color;
	Button_struct->str = str;
}

/* Function to draw a box with rounded corners */
void Button_Draw(Button_t *Button_struct)
{
	coord_t x1, x2, y1, y2;
	coord_t size_x, size_y;

	(void)OSMutexAcquire(GUIMutex);

	x1 = Button_struct->x + Button_struct->radius;
	x2 = Button_struct->x + Button_struct->dx - Button_struct->radius;
	y1 = Button_struct->y + Button_struct->radius;
	y2 = Button_struct->y + Button_struct->dy - Button_struct->radius;

	// Draw the corners
	gdispFillCircle(x1,y1,Button_struct->radius,Button_struct->bg_color);
	gdispFillCircle(x2,y1,Button_struct->radius,Button_struct->bg_color);
	gdispFillCircle(x1,y2,Button_struct->radius,Button_struct->bg_color);
	gdispFillCircle(x2,y2,Button_struct->radius,Button_struct->bg_color);

	// Draw the inner rectangles.
	gdispFillArea(Button_struct->x,y1,Button_struct->radius,Button_struct->dy-2*Button_struct->radius,Button_struct->bg_color);
	gdispFillArea(x2,y1,Button_struct->radius+1,Button_struct->dy-2*Button_struct->radius,Button_struct->bg_color);
	gdispFillArea(x1,Button_struct->y,Button_struct->dx-2*Button_struct->radius,Button_struct->dy+1,Button_struct->bg_color);

	// Obtaining the size of the text.
	size_x = gdispGetStringWidth(Button_struct->str, &fontLarger);
	size_y = gdispGetFontMetric(&fontLarger, fontHeight) -
			gdispGetFontMetric(&fontLarger, fontDescendersHeight);

	// Draw the string in the middle of the box.
	gdispDrawString(Button_struct->x+(Button_struct->dx-size_x)/2,
			Button_struct->y+(Button_struct->dy-size_y)/2,
			Button_struct->str, &fontLarger, Button_struct->font_color);

	(void)OSMutexRelease(GUIMutex);
}


/* Function to draw a box with rounded corners */
void Button_Click(Button_t *Button_struct)
{
	  Button_t Button_backup;
	  Button_backup = *Button_struct;

	  (void)OSMutexAcquire(GUIMutex);

	  // Linha de cima
	  gdispFillArea(Button_struct->x,Button_struct->y,Button_struct->dx,2,GuiBackground);
	  //Linha da esquerda
	  gdispFillArea(Button_struct->x,Button_struct->y,2,Button_struct->dy,GuiBackground);
	  //Quadrado do canto superior esquerdo
	  gdispFillArea(Button_struct->x,Button_struct->y,Button_struct->radius,Button_struct->radius,GuiBackground);
	  //Quadrado do canto inferior esquerdo
	  gdispFillArea(Button_struct->x,(Button_struct->y)+(Button_struct->dy)-12, Button_struct->radius,Button_struct->radius,GuiBackground);
	  // Linha de baixo
	  gdispFillArea(Button_struct->x,(Button_struct->y)+(Button_struct->dy)-2,Button_struct->dx,4,GuiBackground);
	  // Linha da direita
	  gdispFillArea((Button_struct->x)+ (Button_struct->dx)-2,Button_struct->y,3,Button_struct->dy,GuiBackground);
	  //Quadrado do canto superior direito
	  gdispFillArea((Button_struct->x)+(Button_struct->dx)-12,Button_struct->y,Button_struct->radius,Button_struct->radius,GuiBackground);
	  //Quadrado do canto inferior esquerdo
	  gdispFillArea((Button_struct->x)+(Button_struct->dx)-12,(Button_struct->y)+(Button_struct->dy)-12,Button_struct->radius,Button_struct->radius,GuiBackground);

      (void)OSMutexRelease(GUIMutex);

	  // Draw pressed button
	  Button.Update((coord_t)((Button_struct->x)+2),(coord_t)((Button_struct->y)+2),(coord_t)((Button_struct->dx)-4),
			  (coord_t)((Button_struct->dy)-4),(Button_struct->radius),(Button_struct->bg_color),(color_t)0x7BEF,
			  (Button_struct->str),&Button_backup);
	  Button.Draw(&Button_backup);
}


#endif




#if (USE_SLIDER == TRUE)
/* Initializes the Button structure */
void Slider_Init(coord_t x, coord_t y, coord_t width, coord_t height,
		color_t border_color, color_t fg_color, int value,
		Slider_t *Slider_struct, Callbacks click_event)
{
	Slider_struct->x = x;
	Slider_struct->y = y;
	Slider_struct->dx = width;
	Slider_struct->dy = height;
	Slider_struct->radius = 4;
	Slider_struct->border_color = border_color;
	Slider_struct->fg_color = fg_color;
	Slider_struct->value = value;

	// Slider bar area
	Slider_struct->x1 = (Slider_struct->x)+4;
	Slider_struct->y1 = (Slider_struct->y)+4;
	Slider_struct->x2 = (Slider_struct->dx)-8;
	Slider_struct->y2 = (Slider_struct->dy)-7;

	// Event handler
	Slider_struct->event.x1 = x+1;
	Slider_struct->event.y1 = y+1;
	Slider_struct->event.x2 = x+width-2;
	Slider_struct->event.y2 = y+height-2;

	GUI_IncludeObjectIntoEventList(&(Slider_struct->event));

	Slider_struct->event.object = SLIDER_OBJECT;
	Slider_struct->event.ObjectPointer = (void*)Slider_struct;
	Slider_struct->ClickEvent = click_event;
}


/* Initializes the Button structure */
void Slider_Update(int value, Slider_t *Slider_struct)
{
	Slider_struct->value = value;
}

/* Function to draw a box with rounded corners */
void Slider_Draw(Slider_t *Slider_struct)
{
	coord_t x1, x2, y1, y2;

	(void)OSMutexAcquire(GUIMutex);

	x1 = Slider_struct->x + Slider_struct->radius;
	x2 = Slider_struct->x + Slider_struct->dx - Slider_struct->radius;
	y1 = Slider_struct->y + Slider_struct->radius;
	y2 = Slider_struct->y + Slider_struct->dy - Slider_struct->radius;

	// Draw the corners
	gdispFillCircle(x1,y1,Slider_struct->radius,Slider_struct->border_color);
	gdispFillCircle(x2,y1,Slider_struct->radius,Slider_struct->border_color);
	gdispFillCircle(x1,y2,Slider_struct->radius,Slider_struct->border_color);
	gdispFillCircle(x2,y2,Slider_struct->radius,Slider_struct->border_color);

	// Draw the inner rectangles.
	gdispFillArea(Slider_struct->x,y1,Slider_struct->radius,Slider_struct->dy-2*Slider_struct->radius,Slider_struct->border_color);
	gdispFillArea(x2,y1,Slider_struct->radius+1,Slider_struct->dy-2*Slider_struct->radius,Slider_struct->border_color);
	gdispFillArea(x1,Slider_struct->y,Slider_struct->dx-2*Slider_struct->radius,Slider_struct->dy+1,Slider_struct->border_color);

	// Draw background border
	gdispFillArea(Slider_struct->x+2,Slider_struct->y+2,(Slider_struct->dx)-3,(Slider_struct->dy)-3,GuiBackground);

	// Draw slider bar
	gdispFillArea(Slider_struct->x1,Slider_struct->y1,((Slider_struct->x2)*((Slider_struct->value)+1))/100,Slider_struct->y2,Slider_struct->fg_color);

	Slider_struct->lvalue = ((Slider_struct->x2)*((Slider_struct->value)+1))/100;

	(void)OSMutexRelease(GUIMutex);
}


/* Function to draw a box with rounded corners */
void Slider_Click(Slider_t *Slider_struct)
{
	  	coord_t update;

		(void)OSMutexAcquire(GUIMutex);

		update = ((Slider_struct->x2)*(Slider_struct->value))/100;

		// Draw background border
		if (update < Slider_struct->lvalue)
		{
			gdispFillArea(update+(Slider_struct->x1),(Slider_struct->y)+2,(Slider_struct->lvalue)-update+1,(Slider_struct->dy)-3,GuiBackground);
		}

		// Draw slider bar
		if (update > Slider_struct->lvalue)
		{
			gdispFillArea((Slider_struct->lvalue)+(Slider_struct->x1),Slider_struct->y1,update-(Slider_struct->lvalue)+1,Slider_struct->y2,Slider_struct->fg_color);
		}
		Slider_struct->lvalue = update;

		(void)OSMutexRelease(GUIMutex);
}

#endif


#if (USE_CHECKBOX == TRUE)
/* Initializes the Button structure */
void Checkbox_Init(coord_t x, coord_t y, coord_t width, coord_t height,
		color_t border_color, color_t fg_color, unsigned char value,
		Checkbox_t *Checkbox_struct, Callbacks click_event)
{
	Checkbox_struct->x = x;
	Checkbox_struct->y = y;
	Checkbox_struct->dx = width;
	Checkbox_struct->dy = height;
	Checkbox_struct->radius = 4;
	Checkbox_struct->border_color = border_color;
	Checkbox_struct->fg_color = fg_color;
	Checkbox_struct->value = value;

	// Event handler
	Checkbox_struct->event.x1 = x+1;
	Checkbox_struct->event.y1 = y+1;
	Checkbox_struct->event.x2 = x+width-2;
	Checkbox_struct->event.y2 = y+height-2;

	GUI_IncludeObjectIntoEventList(&(Checkbox_struct->event));

	Checkbox_struct->event.object = CHECKBOX_OBJECT;
	Checkbox_struct->event.ObjectPointer = (void*)Checkbox_struct;
	Checkbox_struct->ClickEvent = click_event;
}


/* Initializes the Button structure */
void Checkbox_Update(int value, Checkbox_t *Checkbox_struct)
{
	Checkbox_struct->value = value;
}

/* Function to draw a box with rounded corners */
void Checkbox_Draw(Checkbox_t *Checkbox_struct)
{
	coord_t x1, x2, y1, y2;

	(void)OSMutexAcquire(GUIMutex);

	x1 = Checkbox_struct->x + Checkbox_struct->radius;
	x2 = Checkbox_struct->x + Checkbox_struct->dx - Checkbox_struct->radius;
	y1 = Checkbox_struct->y + Checkbox_struct->radius;
	y2 = Checkbox_struct->y + Checkbox_struct->dy - Checkbox_struct->radius;

	// Draw the corners
	gdispFillCircle(x1,y1,Checkbox_struct->radius,Checkbox_struct->border_color);
	gdispFillCircle(x2,y1,Checkbox_struct->radius,Checkbox_struct->border_color);
	gdispFillCircle(x1,y2,Checkbox_struct->radius,Checkbox_struct->border_color);
	gdispFillCircle(x2,y2,Checkbox_struct->radius,Checkbox_struct->border_color);

	// Draw the inner rectangles.
	gdispFillArea(Checkbox_struct->x,y1,Checkbox_struct->radius,Checkbox_struct->dy-2*Checkbox_struct->radius,Checkbox_struct->border_color);
	gdispFillArea(x2,y1,Checkbox_struct->radius+1,Checkbox_struct->dy-2*Checkbox_struct->radius,Checkbox_struct->border_color);
	gdispFillArea(x1,Checkbox_struct->y,Checkbox_struct->dx-2*Checkbox_struct->radius,Checkbox_struct->dy+1,Checkbox_struct->border_color);

	// Draw background border
	gdispFillArea(Checkbox_struct->x+2,Checkbox_struct->y+2,(Checkbox_struct->dx)-3,(Checkbox_struct->dy)-3,GuiBackground);

	(void)OSMutexRelease(GUIMutex);

	// If true, draw the checkbox mark
	if (Checkbox_struct->value == TRUE)
	{
		Checkbox.Click(Checkbox_struct);
	}
}

/* Function to draw a box with rounded corners */
void Checkbox_Click(Checkbox_t *Checkbox_struct)
{
	(void)OSMutexAcquire(GUIMutex);

	if (Checkbox_struct->value == TRUE)
	{
		// Draw the checkbox left triangle.
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1), (Checkbox_struct->y)+(Checkbox_struct->dy)-3, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-1, (Checkbox_struct->y)+(Checkbox_struct->dy)-3, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-2, (Checkbox_struct->y)+(Checkbox_struct->dy)-3, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-1, (Checkbox_struct->y)+(Checkbox_struct->dy)-4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1), (Checkbox_struct->y)+(Checkbox_struct->dy)-4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1)+1, (Checkbox_struct->y)+(Checkbox_struct->dy)-5, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+4, (Checkbox_struct->y)+((Checkbox_struct->dy)>>1)+2, (Checkbox_struct->x)+((Checkbox_struct->dx)>>1), (Checkbox_struct->y)+(Checkbox_struct->dy)-5, Checkbox_struct->fg_color);

		// Draw the checkbox right triangle.
		gdispDrawLine((Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-3, (Checkbox_struct->y)+(Checkbox_struct->dy)-5, (Checkbox_struct->x)+(Checkbox_struct->dx)-3, (Checkbox_struct->y)+4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-3, (Checkbox_struct->y)+(Checkbox_struct->dy)-4, (Checkbox_struct->x)+(Checkbox_struct->dx)-3, (Checkbox_struct->y)+4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-3, (Checkbox_struct->y)+(Checkbox_struct->dy)-3, (Checkbox_struct->x)+(Checkbox_struct->dx)-3, (Checkbox_struct->y)+4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-2, (Checkbox_struct->y)+(Checkbox_struct->dy)-3, (Checkbox_struct->x)+(Checkbox_struct->dx)-3, (Checkbox_struct->y)+4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+((Checkbox_struct->dx)>>1)-1, (Checkbox_struct->y)+(Checkbox_struct->dy)-3, (Checkbox_struct->x)+(Checkbox_struct->dx)-3, (Checkbox_struct->y)+4, Checkbox_struct->fg_color);
		gdispDrawLine((Checkbox_struct->x)+((Checkbox_struct->dx)>>1), (Checkbox_struct->y)+(Checkbox_struct->dy)-3, (Checkbox_struct->x)+(Checkbox_struct->dx)-3, (Checkbox_struct->y)+4, Checkbox_struct->fg_color);
	}
	else
	{
		// Draw background border
		gdispFillArea(Checkbox_struct->x+2,Checkbox_struct->y+2,(Checkbox_struct->dx)-3,(Checkbox_struct->dy)-3,GuiBackground);
	}

	(void)OSMutexRelease(GUIMutex);
}

#endif


#if (USE_GRAPH == TRUE)
/* Initializes the Button structure */
void Graph_Init(coord_t x, coord_t y, coord_t width, coord_t height,
		int maximum_x, int minimum_y, int maximum_y, unsigned char refresh_bar,
		color_t border_color, color_t fg_color, Trace_t *traces, int ntraces,
		char *title, char *axisx, char *axisy,
		Graph_t *Graph_struct, Callbacks click_event)
{
	coord_t size_x,size_y,x1,x2;
	char	text[8];
	char	*ptext;

	Graph_struct->x = x;
	Graph_struct->y = y;
	Graph_struct->dx = width;
	Graph_struct->dy = height;
	Graph_struct->radius = 4;
	Graph_struct->axis = 0;
	Graph_struct->min_y = minimum_y;
	Graph_struct->max_y = maximum_y;
	Graph_struct->max_x = maximum_x;
	Graph_struct->refresh_bar = refresh_bar;
	Graph_struct->traces = traces;
	Graph_struct->ntraces = ntraces;
	Graph_struct->border_color = border_color;
	Graph_struct->fg_color = fg_color;

	Graph_struct->title_str = title;
	Graph_struct->axisx_str = axisx;
	Graph_struct->axisy_str = axisy;

	PrintDecimal(Graph_struct->max_y, text);
	ptext = text;
	while(*ptext)
	{
		if (*ptext != ' ')
			break;
		else
			ptext++;
	}

	size_x = gdispGetStringWidth(ptext, &fontLarger);

	PrintDecimal(Graph_struct->min_y, text);
	ptext = text;
	while(*ptext)
	{
		if (*ptext != ' ')
			break;
		else
			ptext++;
	}

	size_y = gdispGetStringWidth(ptext, &fontLarger);

	// Verifies maximum text size for axis y limits
	if (size_y > size_x) size_x = size_y = 0;


	// Graph area
	if (size_x <= 15)
	{
		x1 = (Graph_struct->x)+15;
		x2 = (Graph_struct->dx)-30;
	}else
	{
		if (size_x <= 30)
		{
			x1 = (Graph_struct->x)+30;
			x2 = (Graph_struct->dx)-45;
		}else
		{
			if (size_x <= 45)
			{
				x1 = (Graph_struct->x)+45;
				x2 = (Graph_struct->dx)-60;
			}else
			{
				x1 = (Graph_struct->x)+60;
				x2 = (Graph_struct->dx)-75;
			}
		}
	}

	Graph_struct->x1 = x1;
	Graph_struct->x2 = x2;
	Graph_struct->y1 = (Graph_struct->y)+15;
	Graph_struct->y2 = (Graph_struct->dy)-30;

	#if 0
	// Event handler
	Graph_struct->event.x1 = x+3;
	Graph_struct->event.y1 = y+4;
	Graph_struct->event.x2 = x+width-3;
	Graph_struct->event.y2 = y+height-6;

	GUI_IncludeObjectIntoEventList(&(Graph_struct->event));

	Graph_struct->event.object = SLIDER_OBJECT;
	Graph_struct->event.ObjectPointer = (void*)Graph_struct;
	Graph_struct->ClickEvent = click_event;
	#endif
}


/* Graph_AddTraceData */
void Graph_AddTraceData(Graph_t *Graph_struct, int *data)
{
	int value = 0;
	int valuex = 0;
	int finish = 0;
	int y_position = (Graph_struct->y1)+(Graph_struct->y2)-1;
	int n = Graph_struct->ntraces;
	Trace_t *traces = Graph_struct->traces;

	(void)OSMutexAcquire(GUIMutex);

	Graph_struct->axis++;

	// Computes the graph axis x position
	valuex = ((Graph_struct->x2) * (Graph_struct->axis)) / (Graph_struct->max_x);

	if (Graph_struct->axis > Graph_struct->max_x)
	{
		if (valuex > ((Graph_struct->x2)+1))
		{
			valuex = (Graph_struct->x2 + 1);
		}

		// Clear last update line
		gdispFillArea((Graph_struct->x1)+(Graph_struct->last_x), Graph_struct->y1, valuex-(Graph_struct->last_x), Graph_struct->y2, GuiBackground);

		finish = 1;
	}else
	{
		if (Graph_struct->refresh_bar == TRUE)
		{
			// Draw update line
			gdispDrawLine((Graph_struct->x1)+valuex, Graph_struct->y1, (Graph_struct->x1)+valuex, y_position, Graph_struct->fg_color);
		}

		// Clear last update line
		if (Graph_struct->axis == 1)
		{
			gdispFillArea(Graph_struct->x1, Graph_struct->y1, valuex, Graph_struct->y2, GuiBackground);
		}else
		{
			gdispFillArea((Graph_struct->x1)+(Graph_struct->last_x), Graph_struct->y1, valuex-(Graph_struct->last_x), Graph_struct->y2, GuiBackground);
		}
		Graph_struct->last_x = valuex;
	}

	// Draw traces
	while(n--)
	{
		// Computes data place inside the graph
		value = (int)(((*data)*(Graph_struct->y2)) / Graph_struct->max_y);

		// If data is inside graph area
		if ((value >= Graph_struct->min_y) && (value <= Graph_struct->max_y))
		{
			// Determine the trace type
			switch(traces->line_type)
			{
				case POINTS:
					// Plot traces
					gdispDrawPixel((Graph_struct->x1)+valuex-1, y_position - value, traces->color);
				break;

				case LINE:
					// Plot traces
					if (Graph_struct->axis == 1)
					{
						gdispDrawLine((Graph_struct->x1)+valuex-1, y_position - value,(Graph_struct->x1)+(Graph_struct->axis)-1, y_position - value, traces->color);
					}else
					{
						gdispDrawLine(traces->last_x, traces->last_y,(Graph_struct->x1)+valuex-1, y_position - value, traces->color);
					}
					traces->last_x = (Graph_struct->x1)+valuex-1;
					traces->last_y = y_position - value;
				break;

				default:
				break;
			}
		}
		traces++;
		data++;
	}

	// Return to the graph init
	if (finish)
	{
		Graph_struct->axis = 0;
	}

	(void)OSMutexRelease(GUIMutex);
}

/* Function to draw a box with rounded corners */
void Graph_Draw(Graph_t *Graph_struct)
{
	coord_t x1, x2, y1, y2;
	coord_t size_x;
	char	text[8];

	(void)OSMutexAcquire(GUIMutex);

	x1 = Graph_struct->x + Graph_struct->radius;
	x2 = Graph_struct->x + Graph_struct->dx - Graph_struct->radius;
	y1 = Graph_struct->y + Graph_struct->radius;
	y2 = Graph_struct->y + Graph_struct->dy - Graph_struct->radius;

	// Draw the corners
	gdispFillCircle(x1,y1,Graph_struct->radius,Graph_struct->border_color);
	gdispFillCircle(x2,y1,Graph_struct->radius,Graph_struct->border_color);
	gdispFillCircle(x1,y2,Graph_struct->radius,Graph_struct->border_color);
	gdispFillCircle(x2,y2,Graph_struct->radius,Graph_struct->border_color);

	// Draw the inner rectangles.
	gdispFillArea(Graph_struct->x,y1,Graph_struct->radius,Graph_struct->dy-2*Graph_struct->radius,Graph_struct->border_color);
	gdispFillArea(x2,y1,Graph_struct->radius+1,Graph_struct->dy-2*Graph_struct->radius,Graph_struct->border_color);
	gdispFillArea(x1,Graph_struct->y,Graph_struct->dx-2*Graph_struct->radius,Graph_struct->dy+1,Graph_struct->border_color);

	// Draw background border
	gdispFillArea(Graph_struct->x+2,Graph_struct->y+2,(Graph_struct->dx)-3,(Graph_struct->dy)-3,GuiBackground);

	// Draw inside border
	gdispFillArea((Graph_struct->x1)-1,(Graph_struct->y1)-1,(Graph_struct->x2)+3,(Graph_struct->y2)+2,Graph_struct->border_color);

	// Draw inside graph
	gdispFillArea((Graph_struct->x1),(Graph_struct->y1),(Graph_struct->x2)+1,(Graph_struct->y2),GuiBackground);

	if (Graph_struct->title_str != NULL)
	{
		// Obtaining the size of the text
		size_x = gdispGetStringWidth(Graph_struct->title_str, &fontLarger);

		// Draw the title string in the middle of the box.
		gdispDrawString(Graph_struct->x+(Graph_struct->dx-size_x)/2,
				Graph_struct->y+3,
				Graph_struct->title_str, &fontLarger, Graph_struct->fg_color);
	}

	if (Graph_struct->axisx_str != NULL)
	{
		// Obtaining the size of the text
		size_x = gdispGetStringWidth(Graph_struct->axisx_str, &fontLarger);

		// Draw the title string in the middle of the box.
		gdispDrawString(Graph_struct->x+(Graph_struct->dx-size_x)/2,
				(Graph_struct->y)+(Graph_struct->dy) - gdispGetStringHeight(&fontLarger) - 1,
				Graph_struct->axisx_str, &fontLarger, Graph_struct->fg_color);
	}

	if (Graph_struct->axisy_str != NULL)
	{
		// Obtaining the size of the text
		size_x = gdispGetStringWidth(Graph_struct->axisy_str, &fontLarger);

		// Draw the title y axis string
		gdispDrawStringInv(Graph_struct->x+3, ((Graph_struct->y)+(Graph_struct->dy))-(((Graph_struct->dy)-size_x)/2), Graph_struct->axisy_str, &fontLarger, Graph_struct->fg_color);
	}

	// Print axis x limits
	gdispDrawString((Graph_struct->x1)-6,
						(Graph_struct->y)+(Graph_struct->dy) - gdispGetStringHeight(&fontLarger) - 1,
						"0", &fontLarger, Graph_struct->fg_color);

	PrintDecimal(Graph_struct->max_x, text);
	size_x = gdispGetStringWidth(text, &fontLarger);
	gdispDrawString((Graph_struct->x)+(Graph_struct->dx) - size_x - 3,
					(Graph_struct->y)+(Graph_struct->dy) - gdispGetStringHeight(&fontLarger) - 1,
					text, &fontLarger, Graph_struct->fg_color);

	// Print axis y limits
	PrintDecimal(Graph_struct->max_y, text);
	size_x = gdispGetStringWidth(text, &fontLarger);
	gdispDrawString((Graph_struct->x1)-size_x-3 , (Graph_struct->y)+9, text, &fontLarger, Graph_struct->fg_color);

	PrintDecimal(Graph_struct->min_y, text);
	size_x = gdispGetStringWidth(text, &fontLarger);
	gdispDrawString((Graph_struct->x1)-size_x-3 , (Graph_struct->y)+(Graph_struct->dy) - (2*gdispGetStringHeight(&fontLarger)) - 2, text, &fontLarger, Graph_struct->fg_color);


	(void)OSMutexRelease(GUIMutex);
}
#endif

