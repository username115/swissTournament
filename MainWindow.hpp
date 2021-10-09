/*
 * MIT License
 *
 * Copyright (c) 2021 Dan Logan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <memory>
#include "ui_MainWindow.h"

#include "player.hpp"
#include "match.hpp"
#include <QList>
#include <QStringListModel>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow()
    {
        m_ui = std::make_unique<Ui::MainWindow>();
        m_ui->setupUi(this);
        setupWindow();
    }

    void setupWindow();

public slots:
    void addPlayer();
    void removePlayer();
    void editPlayerName();
    void generateMatch(int matchNum);
    void calcFinalResult();

    void updatePlayerList();

    void updatePlayerCount();

    void save();
    void load();

private:
    std::unique_ptr<Ui::MainWindow> m_ui;

    QList<std::shared_ptr<Player>> m_players;
    QStringListModel m_playerList;
    QList<Match> m_matches;

    std::int32_t m_matchCount = 0;
};
