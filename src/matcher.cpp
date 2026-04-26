#include <cstdint>
#include "matcher.h"
#include "order_book.h"

LimitOrderResult limitOrder(OrderBook& order_book, const Order& order)
{
    auto best_level = order.side == Side::BID ? order_book.getBestAskLevel() : order_book.getBestBidLevel();
    if (best_level && best_level->isMatch(order.price))
    {
        TradeAccumulator trade_acc = order_book.tradeQuantity(order);
        auto remaining_quantity = trade_acc.remaining_quantity();
        if (remaining_quantity > 0)
        {
            auto remaining_order = Order(order.id, order.price, remaining_quantity, order.side);
            order_book.add(remaining_order);
            return PartiallyFilled(trade_acc.total_executed, remaining_quantity, std::move(trade_acc.trades));
        } else {
            return Filled(trade_acc.total_executed, std::move(trade_acc.trades));
        }
    } else
    {
        order_book.add(order);
        return OpenResult(order.remaining_quantity);
    }
};

MarketOrderResult marketOrder(OrderBook& order_book, const OrderId id, const Side side, const Quantity quantity)
{
    auto best_level = side == Side::BID ? order_book.getBestAskLevel() : order_book.getBestBidLevel();
    if (best_level)
    {
        Order order = Order(id, best_level->price, quantity, side);
        TradeAccumulator trade_acc = order_book.tradeQuantity(order);   
        auto remaining_quantity = trade_acc.remaining_quantity();
        if (remaining_quantity > 0)
        {
            auto remaining_order = Order(order.id, order.price, remaining_quantity, order.side);
            return PartiallyFilled(trade_acc.total_executed, remaining_quantity, std::move(trade_acc.trades));
        } else {
            return Filled(trade_acc.total_executed, std::move(trade_acc.trades));
        }
    } else
    {
        return NoLiquidityResult(quantity);
    }
}