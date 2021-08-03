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

#include "player.hpp"
#include <QLocale>

std::uint32_t Player::getMatchScore(std::int32_t maxMatch) const
{
    std::uint32_t score = 0;
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (!m_matchResults[i].played)
            continue;
        if (m_matchResults[i].bye)
        {
            score += 3; //match win awarded for bye
        }
        else if (m_matchResults[i].matchWin)
        {
            score += 3;
        }
        else if (m_matchResults[i].matchTie)
        {
            score++;
        }
    }

    return score;
}

std::uint32_t Player::getGameScore(std::int32_t maxMatch) const
{
    std::uint32_t score = 0;
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (!m_matchResults[i].played)
            continue;
        if (m_matchResults[i].bye)
        {
            score += 6; //2 wins awarded for bye matches
        }
        else
        {
            score += (3 * m_matchResults[i].wins) + m_matchResults[i].ties;
        }
    }

    return score;
}

double Player::getMatchWinPercentage(std::int32_t maxMatch) const
{
    int max = 0;
    int score = getMatchScore();
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (m_matchResults[i].played)
            max += 3;
    }

    double winPer = static_cast<double>(score) / static_cast<double>(max);
    if (winPer < 0.33)
        winPer = 0.33;

    return winPer;
}

double Player::getGameWinPercentage(std::int32_t maxMatch) const
{
    int max = 0;
    int score = getGameScore();
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (m_matchResults[i].played)
        {
            max += (m_matchResults[i].wins + m_matchResults[i].losses + m_matchResults[i].ties) * 3;
        }
    }

    double winPer = static_cast<double>(score) / static_cast<double>(max);
    if (winPer < 0.33)
        winPer = 0.33;

    return winPer;
}

double Player::getOpponentMatchWinPercentage(std::int32_t maxMatch) const
{
    int numOpponents = 0;
    double opponentWinPer = 0.0;
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (m_matchResults[i].played && !m_matchResults[i].bye)
        {
            numOpponents++;
            opponentWinPer += m_matchResults[i].opponent->getMatchWinPercentage();
        }
    }

    opponentWinPer /= static_cast<double>(numOpponents);

    return opponentWinPer;
}

double Player::getOpponentGameWinPercentage(std::int32_t maxMatch) const
{
    int numOpponents = 0;
    double opponentWinPer = 0.0;
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (m_matchResults[i].played && !m_matchResults[i].bye)
        {
            numOpponents++;
            opponentWinPer += m_matchResults[i].opponent->getGameWinPercentage();
        }
    }

    opponentWinPer /= static_cast<double>(numOpponents);

    return opponentWinPer;
}

bool Player::scoresValid(QString *reason, std::int32_t maxMatch) const
{
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    QLocale locale;
    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (!m_matchResults[i].played)
            continue;
        if (m_matchResults[i].bye)
            continue;
        if (m_matchResults[i].opponent == nullptr)
        {
            *reason = tr("Match ") + locale.toString(i) + tr(" marked as played, but no opponent was set.");
            return false;
        }
        auto opp = m_matchResults[i].opponent->getResultsForMatch(i);
        if (m_matchResults[i].wins + opp.wins > 3)
        {
            *reason = tr("Match ") + locale.toString(i) + tr(" has a total of more than 3 wins.");
            return false;
        }
        if (m_matchResults[i].losses + opp.losses > 3)
        {
            *reason = tr("Match ") + locale.toString(i) + tr(" has a total of more than 3 losses.");
            return false;
        }
        if (m_matchResults[i].wins != opp.losses)
        {
            *reason = tr("Match ") + locale.toString(i) + tr(" has a different number of wins than opponent losses.");
            return false;
        }
        if (m_matchResults[i].losses != opp.wins)
        {
            *reason = tr("Match ") + locale.toString(i) + tr(" has a different number of losses than opponent wins.");
            return false;
        }
        if (m_matchResults[i].ties != opp.ties)
        {
            *reason = tr("Match ") + locale.toString(i) + tr(" has a different number of ties than opponent ties.");
            return false;
        }
    }

    return true;
}

QList<std::int32_t> Player::getPreviousOpponents(std::int32_t maxMatch) const
{
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    QList<std::int32_t> opp;
    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (m_matchResults[i].opponent != nullptr)
            opp.push_back(m_matchResults[i].opponent->getId());
    }
    return opp;
}

int Player::receivedByes(std::int32_t maxMatch) const
{
    std::int32_t byeCount = 0;
    std::int32_t maxMatchNum = 0;
    if (maxMatch < 0)
        maxMatchNum = m_matchResults.size() - 1;
    else
        maxMatchNum = std::min(maxMatch, static_cast<std::int32_t>(m_matchResults.size()) - 1);

    for (std::int32_t i = 0; i <= maxMatchNum; i++)
    {
        if (m_matchResults[i].played && m_matchResults[i].bye)
            byeCount++;
    }
    return byeCount;
}

MatchResult Player::getResultsForMatch(std::int32_t matchNum) const
{
    if (m_matchResults.size() < (matchNum + 1))
        return MatchResult{};
    return m_matchResults[matchNum];
}

double Player::getTiebrokenScore(std::int32_t maxMatch) const
{
    return static_cast<double>(getMatchScore(maxMatch)) +
           static_cast<double>(getGameScore(maxMatch) + 1) / 100 +      //add 1 to prevent match win from overflowing
           (getMatchWinPercentage(maxMatch) + 0.01) / 100 +             //gives a value between 0 and 1, so it's already a fraction
           (getGameWinPercentage(maxMatch) + 0.01) / 10000 +            //gives a value between 0 and 1, so it's already a fraction
           (getOpponentMatchWinPercentage(maxMatch) + 0.01) / 1000000 + //gives a value between 0 and 1, so it's already a fraction
           (getOpponentGameWinPercentage(maxMatch)) / 100000000;        //gives a value between 0 and 1, so it's already a fraction
}

void Player::setMatchResults(std::int32_t matchNum, const MatchResult &result)
{
    if (m_matchResults.size() < (matchNum + 1))
        m_matchResults.resize(matchNum + 1);
    m_matchResults[matchNum] = result;
    m_matchResults[matchNum].played = true;
}

void Player::setMatchPlayed(std::int32_t matchNum, bool played)
{
    if (m_matchResults.size() < (matchNum + 1))
        m_matchResults.resize(matchNum + 1);
    m_matchResults[matchNum].played = played;
}
