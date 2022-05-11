<meta charset="utf-8" emacsmode="-*- markdown -*-"><link rel="stylesheet" href="https://casual-effects.com/markdeep/latest/apidoc.css?">

                          **6.08 Example Final Project File/Template**

This browser-renderable file is intended to serve as a template for your 6.08 Final Writeupts. We want them to be viewable in a standard browser since it provides a nice uniform viewing experience. This allows you to write in either html or markdwon (easier) but have it be nice and portable since it uses some javascript libraries that it includes.

The file can be written using markdown syntax and then rendered in a browser using the globally-accessible library [Markdeep](https://casual-effects.com/markdeep/). If you've not written in markdown don't worry.  It is actually very simple and designed to not get in your way! Markdown is widely used in documenting your work (github READMEs for example) so even beyond the class it is a great thing to be proficient in.

To view your write-up (as you write it), simply view the file in the browser. To work on it, simply edit it in a any sort of text editor (sublime or whatever your favorite is). Some of them like Sublime even have Markdown plugins to help you.

Markdeep, the particular implementation of Markdown that this page uses, has a fantastic demo page found [here](https://casual-effects.com/markdeep/features.md.html). If you control-click/right-click on that page in your browser and select "View Page Soure", it'll bring up the Markdown that generates all the variou stylings. You can search around on that page as needed.

# Example

Text/Text text.

Include videos of your working system embedded in the document. For example:

![My working project](https://www.youtube.com/watch?v=fy2_qnMnlx4)

 Address each of the specifications listed and how you approached/solved them. Highlight what you did so it easy for a staff member to say, "Oh yeah they have achieved the specifications." For example, maybe structure the report like the following (using some examples from Week 1's watch Design Exercise):

*****************************************************************
*                                                               *
*              .-------.                                        *
*             |  Start  |                                       *
*             |  State  +----.                                  *
*              '-------'      |                                 *
*                             |                                 *
*                             v                                 *
*                           .-+-----.                           *
*                    .---->+  State  |                          *
*                   |      |    0    |                          *
*                   |       '---+---'                           *
*                   |           |                               *
*      getInput()==2|           | getInput()==5                 *
*                   |           v                               *
*                   |       .---+---.                           *
*                   |      |  State  |                          *
*                   |      |    1    |                          *
*                   |       '-+-----'                           *
*                   |         |                                 *
*                    '-------'                                  *
*                                                               *
*                                                               *
*****************************************************************

Do you need some math to explain what you did?  Don't worry, you can do in-line math like $x=5$ or centered math like:
$$
y[n] = x[n-1]+0.3x[n-3]
$$

Questions?  Of course always post on Piazza.


Want to add some *small* code snippets? Totally feel free like below:

My main file is here:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(){
  //stuff
}

void loop(){

  //stuff
}

int other_function(){

}

//etc...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A supporting file is this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int another_function(){

}

//etc...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<style class="fallback">body{visibility:hidden}</style><script>markdeepOptions={tocStyle:'medium'};</script>
<!-- Markdeep: --><script src="https://casual-effects.com/markdeep/latest/markdeep.min.js?" charset="utf-8"></script>
