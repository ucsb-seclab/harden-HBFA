from distutils.filelist import FileList
from pickletools import uint8
import sys
from tkinter.ttk import Treeview
from antlr4 import*
from VfrCompiler.VfrSyntaxLexer import VfrSyntaxLexer
from VfrCompiler.VfrSyntaxParser import VfrSyntaxParser
from VfrCompiler.VfrSyntaxVisitor import VfrSyntaxVisitor
from VfrCompiler.VfrError import *

def VfrParse(Infile, YamlOutFile, JsonOutFile):
    gCVfrErrorHandle.SetInputFile (Infile)
    InputStream = FileStream(Infile)
    Lexer = VfrSyntaxLexer(InputStream)
    Stream = CommonTokenStream(Lexer)
    Parser = VfrSyntaxParser(Stream)
    Tree = Parser.vfrProgram()
    Visitor = VfrSyntaxVisitor()
    Visitor.visit(Tree)
    Visitor.DumpYaml(Visitor.GetRoot(), YamlOutFile)
    Visitor.DumpJson(JsonOutFile)

if __name__ == '__main__':
    Infile = 'VfrCompiler/test.i'
    YamlOutFile = 'VfrCompiler/test.yaml'
    JsonOutFile = 'VfrCompiler/test.json'
    VfrParse(Infile, YamlOutFile, JsonOutFile)
