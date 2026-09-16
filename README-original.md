PROJECT- ASSEMBLER

Hello Roi, you told me to start this README with reminding you that I was in miluim the past 3 months.

I decided to create a "two pass assembler" in a way that would be more efficient than simply read from the text twice (Excluding the pre-assembler).
The way i created this assembler, is that the first pass, will be to create all the binary code that i possibly could, and in adition to create a symbol table. Whichever symbol that was defined before its parameter use- in that very same first round, it will already be converted to binary.
Whichever symbol that was not yet defined, will go into a fix list, with an index telling us where it'll evetually need to be placed.
In order to not have to write to a file (extern) from two different passes, I created a list that accumulates all the usage of extern symbols. After the second pass (The Fix), we will write on an extern file all the usage of externs via a linked list.
The reason I chose two create this kind of assembler, and not the classic "reading the text twice", was because i wanted the efficieny, as known (from OS), that reading from a file, is slower than reading from our own structures.
In order to make this easier to handle, I created a structure called Assemstrct which bundles all the most important information as SymbolTable, fixList, array of data and instructions, IC, DC, etc' in order to not have global variables, which were prohibited. All I needed was a pointer to that structure for everything to go smooth and easier.
I decided also to make use of my knowledge of data stuctures, like hash tables and linked lists for efficiency.
I chose to create the macro table and symbol table using hashtables in case there were many, so as to get to each value quickly with a O(1). It may not have been necessary, but I wanted to use it in this assignment, as I didn't have many options to make my knowledge practical due to miluim the past few years.
Using a flag in the struct Assemstrct, it will imply if there was any error, and therefore won't create any files, but will simply print the errors onto the prompter. If the error is in the first pass, it will include the line which the error accured, in reference to the ".am" file. If the error is in the second round- it will state that the error is in the second pass.
I have left comments mostly in the header files, above where each function is defined. I thought it would be more clear, and will let the code be more flowing. to make it sometimes a bit easier, I also left some comments in the code itself when I thought it would be helpful. 
Finally, I have put a lot of effort into this project, I hope you see the progress I have made. Thank you.
Gavriel Lederman