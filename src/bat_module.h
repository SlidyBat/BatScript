#pragma once

#include "bat_callable.h"
#include "native_wrapper.h"

namespace Bat
{

class BatModule
{
public:
	void AddNativeRaw( std::string name, BatNativeCallback callback )
    {
        Native native;
        native.name = std::move( name );
        native.callback = std::move( callback );

        m_Natives.push_back( std::move( native ) );
    }

    template <typename T>
    void AddNative( std::string name, T callback )
    {
        auto x = std::function( callback );
        auto wrapper = MakeNativeWrapper( std::move( x ) );
        AddNativeRaw( std::move( name ), wrapper );
    }

    const BatNativeCallback* FindNative( const std::string& name ) const
    {
        for( const Native& native : m_Natives )
        {
            if( native.name == name )
                return &native.callback;
        }
        
        return nullptr;
    }
private:
    struct Native
    {
        std::string name;
        BatNativeCallback callback;
    };
    std::vector<Native> m_Natives;
};

}