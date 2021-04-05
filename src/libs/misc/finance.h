//
// Created by mauro on 5/19/20.
//

#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>

namespace utils {

/** \brief Calculate the mathematical expectation of profit
 *
 * If winrate has an invalid value, the function will return 0
 * \param winrate strategy performance (0 to 1.0)
 * \param profit broker payout if successful (usually from 0 to 1.0, but more than 1.0 is possible)
 * \param loss loss in case of defeat (usually always 1.0)
 * \return The mathematical expectation of profit. The value as a percentage of the rate (for example, 0.01 means a profit of 1% of the rate)
 */
    template<class T>
    T calc_expected_payoff(const T &winrate, const T &profit, const T &loss = 1.0) {
        if (winrate > 1.0 || winrate < 0.0) return 0.0;
        return (winrate * profit) - ((1.0 - winrate) * loss);
    }

/** \brief Calculate the minimum strategy effectiveness for a given payout level
 * \param profit broker payout if successful (usually from 0 to 1.0, but more than 1.0 is possible)
 * \param loss loss in case of defeat (usually always 1.0)
 * \return minimum strategy effectiveness
 */
    template<class T>
    T calc_min_winrate(const T &profit, const T &loss = 1.0) {
        return loss / (profit + loss);
    }

/** \brief Calculate the optimal percentage of the deposit rate according to the Kelly criterion
 *
 * If profit is not possible due to low mathematical expectation, will return 0
 * \param winrate strategy performance (0 to 1.0)
 * \param profit broker payout if successful (usually from 0 to 1.0, but more than 1.0 is possible)
 * \param attenuation attenuation coefficient (recommended default value 0.4)
 * \return interest rate (from 0 to 1.0)
 */
    template<class T>
    T calc_kelly_bet(const T &winrate, const T &profit, const T &attenuation = 0.4) {
        if (winrate <= calc_min_winrate(profit)) return 0.0;
        return attenuation * (((profit + 1.0) * winrate - 1.0) / profit);
    }

/** \brief Calculate profit stability
 *
 * This parameter is all the better as the deposit curve is closer to the exponent (for the betting strategy according to the Kelly criterion)
 * The first element of the array must be the initial deposit level (deposit before the first transaction)
 * \param array_depo deposit array
 * \return consistency value
 */
    template<class T1, class T2>
    T1 calc_profit_stability(const T2 &array_depo) {
        size_t size = array_depo.size();
        if (size == 0) return 0.0;
        T1 start_depo = std::log(array_depo.front());
        T1 stop_depo = std::log(array_depo.back());
        T1 delta = (T1) (stop_depo - start_depo) / (T1) (size - 1);
        T1 sum = 0;
        for (size_t i = 1; i < array_depo.size(); ++i) {
            T1 y = start_depo + delta * (T1) i;
            T1 diff = std::log(array_depo[i]) - y;
            sum += diff * diff;
        }
        sum /= (T1) (size - 1);
        return sum;
    }

/** \brief Calculate geometric mean yield
 *
 * The first element of the array must be the initial deposit level (deposit before the first transaction)
 * This function option is suitable for exponential deposit growth.
 * \param array_depo deposit array
 * \return geometric mean yield
 */
    template<class T1, class T2>
    T1 calc_geometric_average_return(const T2 &array_depo) {
        size_t size = array_depo.size();
        if (size < 1) return 0.0;
        T1 mx = 1.0;
        for (size_t i = 1; i < size; i++) {
            T1 ri = array_depo[i - 1] > 0.0 ?
                    1.0 + ((T1) (array_depo[i] - array_depo[i - 1]) / (T1) array_depo[i - 1]) : 0;
            mx *= ri;
        }
        return std::pow(mx, 1.0 / (T1) (size - 1)) - 1.0;
    }

/** \brief Calculate Sharpe Ratio
 *
 * The first element of the array must be the initial deposit level (deposit before the first transaction)
 * This function option is suitable for exponential deposit growth.
 * Sharpe ratio 1 and above - the optimal value of the coefficient,
 * indicating a good strategy or high performance portfolio management
 * \param array_depo deposit array
 * \return Sharpe ratio
 */
    template<class T1, class T2>
    T1 calc_sharpe_ratio(const T2 &array_depo) {
        T1 re = calc_geometric_average_return<T1>(array_depo);
        if (re == 0) return 0.0;
        T1 sum = 0;
        size_t size = array_depo.size();
        for (size_t i = 1; i < size; ++i) {
            T1 ri = array_depo[i - 1] > 0 ?
                    ((T1) (array_depo[i] - array_depo[i - 1]) / (T1) array_depo[i - 1]) : 0;
            T1 diff = ri - re;
            sum += diff * diff;
        }

        if (sum == 0 && re > 0) return std::numeric_limits<T1>::max();
        else if (sum == 0 && re < 0) return std::numeric_limits<T1>::lowest();
        // так как в вычислениях первый элемент - начальный уровень депозита, то size - 2
        sum /= (T1) (size - 2);
        return (re / std::sqrt(sum));
    }

/** \brief Calculate Sharpe Ratio Fast
 *
 * The first element of the array must be the initial deposit level (deposit before the first transaction)
 * This function option is suitable for exponential deposit growth.
 * Sharpe ratio 1 and above - the optimal value of the coefficient,
 * indicating a good strategy or high performance portfolio management
 * \param array_depo deposit array
 * \return Sharpe ratio
 */
    template<class T1, class T2>
    T1 calc_fast_sharpe_ratio(const T2 &array_depo) {
        T1 re = calc_geometric_average_return<T1>(array_depo);
        if (re == 0) return 0.0;
        T1 sum = 0;
        size_t size = array_depo.size();
        for (size_t i = 1; i < size; ++i) {
            T1 ri = array_depo[i - 1] > 0 ?
                    ((T1) (array_depo[i] - array_depo[i - 1]) / (T1) array_depo[i - 1]) : 0;
            T1 diff = ri - re;
            sum += diff * diff;
        }

        if (sum == 0 && re > 0) return std::numeric_limits<T1>::max();
        else if (sum == 0 && re < 0) return std::numeric_limits<T1>::lowest();
        // since in calculations the first element is the initial deposit level, then size - 2
        sum /= (T1) (size - 2);
        return (re * inv_sqrt(sum));
    }

/** \brief Calculate Absolute Balance Drawdown Absolute
 *
 * Absolute balance drawdown (Balance Drawdown Absolute) - the difference between the value of the initial deposit
 * and the minimum value below the initial deposit, to which the balance for the entire history of the account has ever fallen.
 * Absolute drawdown = Initial deposit - Minimum balance.
 * \attention The first element of the curve array must be equal to the initial value of the deposit.
 * \attention The function will return a positive value if there is a drawdown, or 0 if there is no drawdown.
 * \param curve An array of balance curve values
 * \return Absolute Balance Drawdown Absolute
 */
    template<class T1, class T2>
    T1 calc_balance_drawdown_absolute(const T2 &curve) {
        if (curve.size() == 0) return 0.0;
        auto curve_min_it = std::min_element(curve.begin(), curve.end());
        if (curve_min_it == curve.end() || *curve_min_it == curve[0]) return 0.0;
        return curve[0] - *curve_min_it;
    }

/** \brief Calculate the maximum drawdown of balance (Balance Drawdown Maximal)
 *
 * Absolute balance drawdown (Balance Drawdown Absolute) - the difference between the value of the initial deposit
 * and the minimum value below the initial deposit, to which the balance for the entire history of the account has ever fallen.
 * Absolute drawdown = Initial deposit - Minimum balance.
 * \attention The first element of the curve array must be equal to the initial value of the deposit.
 * \attention The function will return a positive value if there is a drawdown, or 0 if there is no drawdown.
 * \param curve An array of balance curve values
 * \return Maximum drawdown of balance (Balance Drawdown Maximal) [money terms,percentage terms]
 */
    template<class T1, class T2>
    std::vector<T1> calc_balance_drawdown_maximal(const T2 &curve) {
        if (curve.size() == 0) return {0,0};
        using NUM_TYPE = typename T2::value_type;
        NUM_TYPE max_value = curve[0];
        NUM_TYPE max_difference = 0;
        NUM_TYPE max_difference_percent = 0;
        std::vector<NUM_TYPE> result{0, 0};
        for (size_t i = 1; i < curve.size(); ++i) {
            if (curve[i] > max_value) max_value = curve[i];
            if (curve[i] < max_value) {
                NUM_TYPE difference = max_value - curve[i];
                if (difference > max_difference) {
                    max_difference = difference;
                    max_difference_percent = difference / max_value;
                }
            }
        }
        result[0] = max_difference;
        result[1] = max_difference_percent;
        return result;
    }

/** \brief Calculate Relative Drawdown Relative
 *
 * Relative balance drawdown (Balance Drawdown Relative) - the largest drop in the balance between the local maximum
 * and the next local minimum in percent.
 * \attention The first element of the curve array must be equal to the initial value of the deposit.
 * \attention The function will return a positive value from 0 to 1 if there is a drawdown, or 0 if there is no drawdown.
 * \param curve an array of balance curve values
 * \return Relative balance drawdown (Balance Drawdown Relative), value from 0.0. up to 1.0
 */
    template<class T1, class T2>
    T1 calc_balance_drawdown_relative(const T2 &curve) {
        if (curve.size() == 0) return 0.0;
        using NUM_TYPE = typename T2::value_type;
        NUM_TYPE max_value = curve[0];
        NUM_TYPE max_difference = 0;
        for (size_t i = 1; i < curve.size(); ++i) {
            if (curve[i] > max_value) max_value = curve[i];
            if (curve[i] < max_value) {
                NUM_TYPE difference = max_value - curve[i];
                difference /= (NUM_TYPE) max_value;
                if (difference > max_difference) max_difference = difference;
            }
        }
        return (T1) max_difference;
    }

    inline double sortinoRatio(double ret, double downsideDeviation, double riskFreeRate) {
        if (downsideDeviation == 0.0) return 0.0;
        return (ret - riskFreeRate) / downsideDeviation;
    }

    inline double calmarRatio(double ret, double mdd, double riskFreeRate) {
        if (mdd == 0.0) return 0.0;
        return (ret - riskFreeRate) / mdd;
    }
}

// --------------------------------
//inline double sharpeRatio(double ret, double vol, double riskFreeRate) {
//    if (vol == 0.0) return 0.0;
//    return (ret - riskFreeRate) / vol;
//}
//

//inline double sharpeRatioVariance(double sharpe, double T) {
//    return (1.0 + 0.5 * sharpe * sharpe) / std::sqrt(T);
//}