#!/usr/bin/python

"""
Author: Alexander Matz
Date: 2013-06-04

Usage:
	./rbvt.py <data-file>
	or
	yourprogram | ./rbvt.py

Your input is read line by line with each line representing one version of your tree.
You can walk through your input by pressing space and exit the program by pressing escape

This script expects your data to follow this syntax:
<key> <color as 'r' or 'b'>;<left child or 'n'>;<right child or 'n'>;

Example input for the following (WRONG) Red Black Tree:
              5r
       2b              7b
           4r      6r        9r
	                   8b

5 r;2 b;n;4 r;n;n;7 b;6 r;n;n;9 r;8 b;n;n;n;


C example to recursively print a red black tree (without a trailing newline):

void print_node(struct node *node)
{
	if (node == NULL) {
		printf("n;");
	} else {
		char color = (node->color == RED ? 'r' : 'b');
		printf("%d %c;", node->key, color);
		print_node(node->left);
		print_node(node->right);
	}
}
"""

import sys
import fileinput
try:
	from Tkinter import * #python 2.x
except:
	from tkinter import * #python 3.x

class RbNode(object):
	def __init__(self, key, red, left=None, right=None):
		self.key = key
		self.red = red
		self.left = left
		self.right = right

class RbtView(Canvas):
	def __init__(self, parent=None):
		Canvas.__init__(self, parent)
		self.bind("<Configure>", self.configureEvent)
		self.initialized = False
		self.tree = None
		self.done = False
	def setTree(self, rbtree):
		self.tree = rbtree
		if (self.initialized):
			self.repaint()
	def resize(self):
		self.w = self.winfo_width()
		self.h = self.winfo_height()
		self.repaint()
	def repaint(self):
		if (self.tree == None):
			return

		def paintNode(node, tier=1, xpos=0):
			color = "red" if node.red else "black"
			fcolor = "black" if node.red else "white"
			x = (xpos/2)*self.w + self.w/2
			y = (tier) * 40
			self.create_oval(x-10, y-10, x+10, y+10, fill=color)
			self.create_text(x, y, text=str(node.key), fill=fcolor)
			if (node.left != None):
				newxpos = xpos - 1. / (2**tier)
				newtier = tier + 1
				tx, ty = paintNode(node.left, newtier, newxpos)
				self.create_line(x,y,tx,ty,tag="line")
			if (node.right != None):
				newxpos = xpos + 1. / (2**tier)
				newtier = tier + 1
				tx, ty = paintNode(node.right, newtier, newxpos)
				self.create_line(x,y,tx,ty,tag="line")
			return x, y

		self.delete("all")
		paintNode(self.tree)
		self.tag_lower("line")
		if (self.done):
			self.create_text(10,10,text="EOF reached",anchor="nw")
	def setDone(self, done):
		self.done = done
		self.repaint()
	def configureEvent(self, event):
		self.resize()
		self.initialized = True


class RbtMainWin(Tk):
	def __init__(self, stream):
		Tk.__init__(self)
		self.view = RbtView(self)
		self.view.pack(fill=BOTH, expand=1)
		self.bind("<Key>", self.keyEvent)
		self.stream = stream
		self.line = None
		self.last_line = None
		self.viewed_last = 0
	def keyEvent(self, event):
		if (event.keysym == "space"):
			if (self.viewed_last == 0):
				self.last_line = self.line
				self.line = self.stream.readline()
			else:
				self.viewed_last = 0
			if (self.line):
				self.view.setTree(parseTree(self.line))
			else:
				self.view.setDone(True)
		if (event.keysym == "BackSpace"):
			self.viewed_last = 1
			if (self.last_line):
				self.view.setTree(parseTree(self.last_line))
			else:
				self.view.setDone(True)
		if (event.keysym == "Escape"):
			sys.exit(0)

def parseTree(data):
	def buildNode(fields, pos):
		vals = fields[pos].split()
		key = int(vals[0])
		red = vals[1] in ["r", "R"]
		pos = pos+1
		if (not fields[pos] in ["n", ""]):
			left, pos = buildNode(fields, pos)
		else:
			left = None
			pos = pos+1
		if (not fields[pos] in ["n", ""]):
			right, pos = buildNode(fields, pos)
		else:
			right = None
			pos = pos+1
		return RbNode(key, red, left, right), pos
	fields = data.split(";")
	tree, dummy = buildNode(fields, 0)
	return tree

def main(argv):
	win = RbtMainWin(fileinput.input())
	win.mainloop()

if __name__ == "__main__":
	main(sys.argv)

