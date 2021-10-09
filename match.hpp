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

#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include "player.hpp"
#include <random>

#include "json.hpp"

struct Matchup
{
    std::shared_ptr<Player> p1;
    std::shared_ptr<Player> p2;
};

class Match : public QObject
{
    Q_OBJECT

public:
    Match() : QObject(){};

    Match(QPushButton *generateMatchB, QTableWidget *matchView) : QObject(), m_rd(), m_reng(m_rd())
    {
        m_generateMatchB = generateMatchB;
        m_matchView = matchView;

        //disable by default
        setEnabled(false);
        setupTables();
    };

    Match(const Match &mch) : QObject(), m_rd(), m_reng(m_rd())
    {
        m_generateMatchB = mch.m_generateMatchB;
        m_matchView = mch.m_matchView;
        m_matchups = mch.m_matchups;
    }

    Match(Match &&mch) : QObject(), m_rd(), m_reng(m_rd())
    {
        m_generateMatchB = mch.m_generateMatchB;
        m_matchView = mch.m_matchView;
        m_matchups = std::move(mch.m_matchups);
    }

    ~Match() = default;

    Match &operator=(const Match &mch)
    {
        m_generateMatchB = mch.m_generateMatchB;
        m_matchView = mch.m_matchView;
        m_matchups = mch.m_matchups;
        return *this;
    }

    Match &operator=(Match &&mch)
    {
        m_generateMatchB = mch.m_generateMatchB;
        m_matchView = mch.m_matchView;
        m_matchups = std::move(mch.m_matchups);
        return *this;
    }

    void setupTables();

    nlohmann::json toJson() const;
    bool loadMatch(const nlohmann::json& j, const QList<std::shared_ptr<Player>>& playerList, std::size_t matchNum);

public slots:
    void setEnabled(bool enable);

    void generateMatch(const QList<std::shared_ptr<Player>> &playerList, std::int32_t matchNum);

    bool finalizeMatch(const QList<std::shared_ptr<Player>> &playerList, std::int32_t matchNum);

private:
    //recursively try pairings
    //requires player list to be sorted based on previous scores
    //matchNum is max match to consider (usually the previous match)
    //maxByes is max previous byes per player for pairing attempt
    //maxMatchups is max previous matchups per player for pairing attempt
    //returns true if pairing found
    bool generatePairing(const QList<std::shared_ptr<Player>> &playerList, std::int32_t matchNum, std::int32_t maxByes, std::int32_t maxMatchups);
    QPushButton *m_generateMatchB = nullptr;
    QTableWidget *m_matchView = nullptr;

    QList<Matchup> m_matchups;

    std::random_device m_rd;
    std::default_random_engine m_reng;

    void updateMatchView();
    void updateMatchResultsView(std::size_t matchNum);
};
