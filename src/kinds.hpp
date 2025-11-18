#ifndef FORMAT_KINDS_HPP
#define FORMAT_KINDS_HPP
enum class NodeKind {
    Program,
    EndProgram,

    Module,
    EndModule,

    Subroutine,
    EndSubroutine,

    Function,
    EndFunction,

    Interface,
    EndInterface,

    Use,
    Call,

    If,
    IfConstruct,
    ElseIf,
    Else,
    EndIf,

    Do,
    EndDo,

    SelectCase,
    Case,
    EndSelect,

    Declaration,
    Assignment,

    Comment,
    Blank,
    Unknown,

    Type,
    EndType
};


#endif //FORMAT_KINDS_HPP