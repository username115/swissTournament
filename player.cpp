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

#include <iostream>

constexpr const char* MR_PLAYED_LBL = "played";
constexpr const char* MR_WIN_LBL = "match_win";
constexpr const char* MR_TIE_LBL = "match_tie";
constexpr const char* MR_BYE_LBL = "bye";
constexpr const char* MR_WINS_LBL = "wins";
constexpr const char* MR_LOSSES_LBL = "losses";
constexpr const char* MR_TIES_LBL = "ties";
constexpr const char* MR_OPP_LBL = "opponent_name";
constexpr const char* P_ID_LBL = "id";
constexpr const char* P_NAME_LBL = "name";
constexpr const char* P_MRS_LBL = "match_results";

void from_json(const nlohmann::json& j, MatchResult& m)
{
    j[MR_PLAYED_LBL].get_to(m.played);
    j[MR_WIN_LBL].get_to(m.matchWin);
    j[MR_TIE_LBL].get_to(m.matchTie);
    j[MR_BYE_LBL].get_to(m.bye);
    j[MR_WINS_LBL].get_to(m.wins);
    j[MR_LOSSES_LBL].get_to(m.losses);
    j[MR_TIES_LBL].get_to(m.ties);

    //store the opponent name to be used for pointer lookup later
    m.opponent = std::make_shared<Player>(QString::fromStdString(j[MR_OPP_LBL]), -1);
}
void to_json(nlohmann::json& j, const MatchResult& m)
{
    j[MR_PLAYED_LBL] = m.played;
    j[MR_WIN_LBL] = m.matchWin;
    j[MR_TIE_LBL] = m.matchTie;
    j[MR_BYE_LBL] = m.bye;
    j[MR_WINS_LBL] = m.wins;
    j[MR_LOSSES_LBL] = m.losses;
    j[MR_TIES_LBL] = m.ties;
    j[MR_OPP_LBL] = (m.opponent != nullptr ? m.opponent->getName().toStdString() : "");
}

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
nlohmann::json Player::toJson() const
{
    std::cout << "here" << std::endl;
    nlohmann::json j;
    j[P_ID_LBL] = m_id;
    j[P_NAME_LBL] = m_name.toStdString();
    j[P_MRS_LBL] = nlohmann::json();

    auto& mrs = j[P_MRS_LBL];
    for (const auto& mr : m_matchResults)
    {
        mrs.push_back(mr);
    }

    std::cout << "out" << std::endl;
    return j;
}
bool Player::load(const nlohmann::json& j)
{
    if (!(j.contains(P_ID_LBL) && j.contains(P_NAME_LBL) && j.contains(P_MRS_LBL)))
    {
        return false;
    }

    j[P_ID_LBL].get_to(m_id);
    m_name = QString::fromStdString(j[P_NAME_LBL]);

    const auto& mrsJ = j[P_MRS_LBL];
    if (!mrsJ.is_null())
    {
        // null is fine, that means there were no matches played
        for (const auto& mr : mrsJ)
        {
            m_matchResults.emplaceBack(); // make an empty match results
            m_matchResults.back() = mr; // use JSON conversion function defined above
        }
    }

    return true;
}
bool Player::finalizeLoad(const QList<std::shared_ptr<Player>>& playerList)
{
    bool res = true;
    for (auto& mr : m_matchResults)
    {
        bool found = false;
        for (const auto& p : playerList)
        {
            if (mr.opponent->getName() == p->getName())
            {
                found = true;
                mr.opponent = p;
                break;
            }
        }

        //if an opponent player couldn't be found print warning and go to next match;
        if (!found)
        {
            std::cerr << "WARNING: opponent lookup failed in match for " << m_name.toStdString() << "\n";
            res = false;
        }
    }

    return res;
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
