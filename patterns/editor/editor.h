#pragma once

#include <string>
#include <memory>
#include <vector>
#include <iostream>

class Command;

class Editor {
public:
    Editor() = default;

    const std::string& GetText() const;

    void Type(char c);

    void ShiftLeft();

    void ShiftRight();

    void Backspace();

    void Undo();

    void Redo();

private:
    void AddCommand(std::unique_ptr<Command> cmd);
    std::vector<std::unique_ptr<Command>> commands_;
    std::vector<std::unique_ptr<Command>>::iterator idx_ = commands_.begin();
    std::string text_ = {};
    size_t cursor_ = 0;

    friend class Command;
};

class Command {
public:
    Command(Editor& editor) : text_(editor.text_), cursor_(editor.cursor_), idx_(editor.idx_) {
    }
    virtual ~Command() = default;

    virtual bool Redo() = 0;
    virtual void Undo() = 0;

protected:
    std::string& text_;
    size_t& cursor_;
    std::vector<std::unique_ptr<Command>>::iterator& idx_;
    char symbol_;
};

class CommandEdit : public Command {
public:
    CommandEdit(Editor& editor) : Command(editor) {
    }
    virtual bool Redo() = 0;
    virtual void Undo() = 0;
    bool Type() {
        text_.insert(cursor_, 1, symbol_);
        cursor_++;
        return true;
    }
    bool BackSpace() {
        if (cursor_) {
            --cursor_;
            symbol_ = text_[cursor_];
            text_.erase(cursor_, 1);
            return true;
        }
        return false;
    }
};

class CommandType : public CommandEdit {
public:
    CommandType(Editor& edit, char c) : CommandEdit(edit) {
        symbol_ = c;
    }

    bool Redo() override {
        return Type();
    }

    void Undo() override {
        BackSpace();
    }
};

class BackspaceCommand : public CommandEdit {

    bool Redo() override {
        return BackSpace();
    }

    void Undo() override {
        Type();
    }

public:
    BackspaceCommand(Editor& editor) : CommandEdit(editor) {
    }
};

class CommandChangeCursor : public Command {
public:
    CommandChangeCursor(Editor& editor) : Command(editor) {
    }
    virtual bool Redo() = 0;
    virtual void Undo() = 0;
    bool ShiftLeft() {
        if (cursor_) {
            --cursor_;
            return true;
        }
        return false;
    }
    bool ShiftRight() {
        if (cursor_ < text_.size()) {
            ++cursor_;
            return true;
        }
        return false;
    }
};
class ShiftLeftCommand : public CommandChangeCursor {
public:
    ShiftLeftCommand(Editor& edit) : CommandChangeCursor(edit) {
    }

private:
    bool Redo() override {
        return ShiftLeft();
    }

    void Undo() override {
        ShiftRight();
    }
};

class ShiftRightCommand : public CommandChangeCursor {
public:
    ShiftRightCommand(Editor& editor) : CommandChangeCursor(editor) {
    }

private:
    bool Redo() override {
        return ShiftRight();
    }

    void Undo() override {
        ShiftLeft();
    }
};

void Editor::Undo() {
    if (idx_ != commands_.begin()) {
        --idx_;
        (*idx_)->Undo();
    }
}

void Editor::Redo() {
    if (idx_ != commands_.end()) {
        (*idx_)->Redo();
        ++idx_;
    }
}

const std::string& Editor::GetText() const {
    return text_;
}

void Editor::Type(char c) {
    AddCommand(std::unique_ptr<Command>(new CommandType(*this, c)));
}

void Editor::AddCommand(std::unique_ptr<Command> cmd) {
    if (cmd->Redo()) {
        commands_.erase(idx_, commands_.end());
        commands_.emplace_back(std::move(cmd));
        idx_ = commands_.end();
    }
}

void Editor::ShiftLeft() {
    AddCommand(std::unique_ptr<Command>(new ShiftLeftCommand(*this)));
}

void Editor::ShiftRight() {
    AddCommand(std::unique_ptr<Command>(new ShiftRightCommand(*this)));
}

void Editor::Backspace() {
    AddCommand(std::unique_ptr<Command>(new BackspaceCommand(*this)));
}
