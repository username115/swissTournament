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

#include <QString>
#include <QObject>
#include <QList>

class Player;

struct MatchResult
{
    bool played = false; //set to true when this match has been played, even if it was a bye. Scores are ignored if false
    bool matchWin = false;
    bool matchTie = false;
    bool bye = false;
    std::uint32_t wins = 0;
    std::uint32_t losses = 0;
    std::uint32_t ties = 0;
    std::shared_ptr<Player> opponent = nullptr;
};

class Player : public QObject
{
    Q_OBJECT
public:
    Player() : QObject() {}

    Player(const QString &name, std::int32_t id) : QObject()
    {
        m_name = name;
        m_id = id;
    }

    Player(const Player &pl)
    {
        m_name = pl.m_name;
        m_id = pl.m_id;
        m_matchResults = pl.m_matchResults;
    }

    Player(Player &&pl)
    {
        m_name = std::move(pl.m_name);
        m_id = std::move(pl.m_id);
        m_matchResults = std::move(pl.m_matchResults);
    }

    ~Player() = default;

    Player &operator=(const Player &pl)
    {
        m_name = pl.m_name;
        m_id = pl.m_id;
        m_matchResults = pl.m_matchResults;
        return *this;
    }

    Player &operator=(Player &&pl)
    {
        m_name = std::move(pl.m_name);
        m_id = std::move(pl.m_id);
        m_matchResults = std::move(pl.m_matchResults);
        return *this;
    }

    std::uint32_t getMatchScore(std::int32_t maxMatch = -1) const;

    std::uint32_t getGameScore(std::int32_t maxMatch = -1) const;

    double getMatchWinPercentage(std::int32_t maxMatch = -1) const;

    double getGameWinPercentage(std::int32_t maxMatch = -1) const;

    double getOpponentMatchWinPercentage(std::int32_t maxMatch = -1) const;

    double getOpponentGameWinPercentage(std::int32_t maxMatch = -1) const;

    bool scoresValid(QString *reason, std::int32_t maxMatch = -1) const;

    MatchResult getResultsForMatch(std::int32_t maxMatch = -1) const;

    QList<std::int32_t> getPreviousOpponents(std::int32_t maxMatch = -1) const;

    std::int32_t receivedByes(std::int32_t maxMatch = -1) const;

    inline QString getName() const
    {
        return m_name;
    }

    inline void setName(const QString &name)
    {
        m_name = name;
    }

    inline std::int32_t getId() const
    {
        return m_id;
    }

    inline void setId(std::int32_t id)
    {
        m_id = id;
    }

    double getTiebrokenScore(std::int32_t maxMatch = -1) const;

public slots:
    void setMatchResults(std::int32_t matchNum, const MatchResult &result);
    void setMatchPlayed(std::int32_t matchNum, bool played);

private:
    std::int32_t m_id = -1;
    QString m_name = "";
    QList<MatchResult> m_matchResults;
};
