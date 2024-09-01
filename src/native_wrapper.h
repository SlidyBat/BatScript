#pragma once

#include "bat_callable.h"
#include "bat_object.h"

namespace Bat
{
template <typename Ret>
BatNativeCallback MakeNativeWrapper( std::function<Ret()> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 0 )
            throw BatObjectError( "Native call argument mismatch" );

        Ret result = callback();
        return ToBatObject<Ret>( result );
    };
}

template <>
inline BatNativeCallback MakeNativeWrapper<void>( std::function<void()> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 0 )
            throw BatObjectError( "Native call argument mismatch" );

        callback();
        return BatObject();
    };
}

template <typename Ret, typename Arg1>
BatNativeCallback MakeNativeWrapper( std::function<Ret(Arg1)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 1 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Ret result = callback( arg1 );
        return ToBatObject<Ret>( result );
    };
}
template <typename Arg1>
BatNativeCallback MakeNativeWrapper( std::function<void(Arg1)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 1 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        callback( arg1 );
        return BatObject();
    };
}

template <typename Ret, typename Arg1, typename Arg2>
BatNativeCallback MakeNativeWrapper( std::function<Ret(Arg1, Arg2)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 2 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg2>( args[1] );
        Ret result = callback( arg1, arg2 );
        return ToBatObject<Ret>( result );
    };
}
template <typename Arg1, typename Arg2>
BatNativeCallback MakeNativeWrapper( std::function<void(Arg1, Arg2)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 2 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg2>( args[1] );
        callback( arg1, arg2 );
        return BatObject();
    };
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3>
BatNativeCallback MakeNativeWrapper( std::function<Ret(Arg1, Arg2, Arg3)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 3 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg2>( args[1] );
        Arg3 arg3 = FromBatObject<Arg3>( args[2] );
        Ret result = callback( arg1, arg2, arg3 );
        return ToBatObject<Ret>( result );
    };
}
template <typename Arg1, typename Arg2, typename Arg3>
BatNativeCallback MakeNativeWrapper( std::function<void(Arg1, Arg2, Arg3)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 3 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg2>( args[1] );
        Arg3 arg3 = FromBatObject<Arg3>( args[2] );
        callback( arg1, arg2, arg3 );
        return BatObject();
    };
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
BatNativeCallback MakeNativeWrapper( std::function<Ret(Arg1, Arg2, Arg3, Arg4)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 4 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg1>( args[1] );
        Arg3 arg3 = FromBatObject<Arg3>( args[2] );
        Arg4 arg4 = FromBatObject<Arg4>( args[3] );
        Ret result = callback( arg1, arg2, arg3, arg4 );
        return ToBatObject<Ret>( result );
    };
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
BatNativeCallback MakeNativeWrapper( std::function<void(Arg1, Arg2, Arg3, Arg4)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 4 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg1>( args[1] );
        Arg3 arg3 = FromBatObject<Arg3>( args[2] );
        Arg4 arg4 = FromBatObject<Arg4>( args[3] );
        callback( arg1, arg2, arg3, arg4 );
        return BatObject();
    };
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
BatNativeCallback MakeNativeWrapper( std::function<Ret(Arg1, Arg2, Arg3, Arg4, Arg5)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 5 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg1>( args[1] );
        Arg3 arg3 = FromBatObject<Arg3>( args[2] );
        Arg4 arg4 = FromBatObject<Arg4>( args[3] );
        Arg5 arg5 = FromBatObject<Arg5>( args[4] );
        Ret result = callback( arg1, arg2, arg3, arg4, arg5 );
        return ToBatObject<Ret>( result );
    };
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
BatNativeCallback MakeNativeWrapper( std::function<void(Arg1, Arg2, Arg3, Arg4, Arg5)> callback )
{
    return [callback]( const std::vector<BatObject>& args ) {
        if( args.size() != 5 )
            throw BatObjectError( "Native call argument mismatch" );

        std::decay_t<Arg1> arg1 = FromBatObject<std::decay_t<Arg1>>( args[0] );
        Arg2 arg2 = FromBatObject<Arg1>( args[1] );
        Arg3 arg3 = FromBatObject<Arg3>( args[2] );
        Arg4 arg4 = FromBatObject<Arg4>( args[3] );
        Arg5 arg5 = FromBatObject<Arg5>( args[4] );
        callback( arg1, arg2, arg3, arg4, arg5 );
        return BatObject();
    };
}

}